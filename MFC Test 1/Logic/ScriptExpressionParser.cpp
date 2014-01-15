#include "stdafx.h"
#include "ScriptExpressionParser.h"

namespace Logic
{
   namespace Scripts
   {
      namespace Compiler
      {
         // -------------------------------- CONSTRUCTION --------------------------------

         /// <summary>Creates a script expression parser</summary>
         /// <param name="begin">Position of first expression token</param>
         /// <param name="end">Position after last expression token</param>
         ScriptExpressionParser::ScriptExpressionParser(TokenIterator& begin, TokenIterator& end)
            : InputBegin(begin), InputEnd(end)
         {
         }


         ScriptExpressionParser::~ScriptExpressionParser()
         {
         }

         // ------------------------------- STATIC METHODS -------------------------------

         // ------------------------------- PUBLIC METHODS -------------------------------

         /// <summary>Parses the expression, ensures it is correct and produces infix/postfix tokens.</summary>
         /// <exception cref="Logic::ArgumentException">Error in parsing algorithm</exception>
         /// <exception cref="Logic::InvalidOperationException">Error in parsing algorithm</exception>
         /// <exception cref="Logic::ScriptSyntaxException">Syntax error in expression</exception>
         void  ScriptExpressionParser::Parse()
         {
            // DEBUG: print input
            Console::Write(L"Input: ");
            for (auto it = InputBegin; it != InputEnd; ++it)
               Console::Write(L"%s ", it->Text.c_str());
            Console::WriteLn(L"");
                         
            try
            {
               // Produce parse tree
               unique_ptr<Expression> tree = unique_ptr<Expression>(ReadExpression(InputBegin));

               // Extract tokens
               tree->getTokenArray(Traversal::InOrder, InfixParams);
               tree->getTokenArray(Traversal::PostOrder, PostfixParams);

               // DEBUG: Print
               Console::WriteLn(L"Output: %s", tree->debugPrint().c_str() );
               //Console::WriteLn(L"PreOrder (Not Used): %s", tree->debugPrintTraversal(Traversal::PreOrder).c_str());
               Console::WriteLn(L"Infix: %s", tree->debugPrintTraversal(Traversal::InOrder).c_str());
               Console::WriteLn(L"Postfix: %s", tree->debugPrintTraversal(Traversal::PostOrder).c_str());
               Console::WriteLn(L"");

            }
            catch (ScriptSyntaxException& e)
            {
               Console::WriteLn(e.Message);
            }
         }

         // ------------------------------ PROTECTED METHODS -----------------------------

         // ------------------------------- PRIVATE METHODS ------------------------------

         /// <summary>Attempts to matches any literal</summary>
         /// <param name="pos">Position of literal</param>
         /// <returns></returns>
         bool  ScriptExpressionParser::MatchLiteral(const TokenIterator& pos)
         {
            // Validate position 
            if (pos < InputEnd) 
               // check type
               switch (pos->Type)
               {
               case TokenType::Variable:
               case TokenType::Constant:
               case TokenType::Number:
               case TokenType::String:
               case TokenType::GameObject:
               case TokenType::ScriptObject:
                  return true;
               }

            return false;
         }

         /// <summary>Attempts to matches a specific operator</summary>
         /// <param name="pos">Position of operator</param>
         /// <returns></returns>
         bool ScriptExpressionParser::MatchOperator(const TokenIterator& pos, const WCHAR* op)
         {
            // Validate position and compare operator
            return pos < InputEnd && pos->Type == TokenType::Operator && pos->Text == op;
         }

         /// <summary>Attempts to matches any operator of a given precedence</summary>
         /// <param name="pos">Position of operator</param>
         /// <returns></returns>
         /// <exception cref="Logic::ArgumentException">Invalid precendence detected</exception>
         bool ScriptExpressionParser::MatchOperator(const TokenIterator& pos, UINT precedence)
         {
            // Validate position. Ensure operator
            if (pos >= InputEnd || pos->Type != TokenType::Operator)
               return false;

            // Precedence table taken from X2 scripting manual
            switch (precedence)
            {
            case 0: return pos->Text == L"OR";
            case 1: return pos->Text == L"AND";
            case 2: return pos->Text == L"|";
            case 3: return pos->Text == L"^";
            case 4: return pos->Text == L"&";
            case 5: return pos->Text == L"==" || pos->Text == L"!=";
            case 6: return pos->Text == L"<" || pos->Text == L">" || pos->Text == L"<=" || pos->Text == L">=";
            case 7: return pos->Text == L"+" || pos->Text == L"-";
            case 8: return pos->Text == L"*" || pos->Text == L"/" || pos->Text == L"mod";
            case 9: return pos->Text == L"~" || pos->Text == L"-" || pos->Text == L"!";
            }

            throw ArgumentException(HERE, L"precedence", GuiString(L"Invalid precedence %d", precedence));
         }

         /// <summary>Reads the current token as a literal</summary>
         /// <param name="pos">Current position</param>
         /// <returns>Token</returns>
         /// <exception cref="Logic::InvalidOperationException">Token is not a literal</exception>
         /// <remarks>Advances the iterator to beyond the literal</remarks>
         const ScriptToken&  ScriptExpressionParser::ReadLiteral(TokenIterator& pos)
         {
            // Validate position/type
            if (pos >= InputEnd || pos->Type == TokenType::Operator || pos->Type == TokenType::Text)
               throw InvalidOperationException(HERE, L"Not a literal");

            // Read literal. Advance position
            return *(pos++);
         }

         /// <summary>Reads the current token as an operator</summary>
         /// <param name="pos">Current position</param>
         /// <returns>Token</returns>
         /// <exception cref="Logic::InvalidOperationException">Token is not a operator</exception>
         /// <remarks>Advances the iterator to beyond the operator</remarks>
         const ScriptToken&  ScriptExpressionParser::ReadOperator(TokenIterator& pos)
         {
            // Verify operator
            if (pos >= InputEnd || pos->Type != TokenType::Operator)
               throw InvalidOperationException(HERE, L"Not an operator");

            // Read operator. Advance position
            return *(pos++);
         }


         /// <summary>Reads an entire expression</summary>
         /// <param name="pos">Position of first token of expression</param>
         /// <returns>Expression tree</returns>
         /// <exception cref="Logic::ArgumentException">Invalid precendence detected</exception>
         /// <exception cref="Logic::InvalidOperationException">Attempted to read incorrect type of Token</exception>
         /// <exception cref="Logic::ScriptSyntaxException">Syntax error</exception>
         /// <remarks>Advances the iterator to beyond the end of the expression</remarks>
         ScriptExpressionParser::Expression*  ScriptExpressionParser::ReadExpression(TokenIterator& pos)
         {
            // Expression = Comparison
            return ReadBinaryExpression(pos, MIN_PRECEDENCE);
         }

         /// <summary>Reads a binary expression, unary expression, sub-expression, or literal</summary>
         /// <param name="pos">Position of first token of expression</param>
         /// <returns>Expression tree</returns>
         /// <exception cref="Logic::ArgumentException">Invalid precendence detected</exception>
         /// <exception cref="Logic::InvalidOperationException">Attempted to read incorrect type of Token</exception>
         /// <exception cref="Logic::ScriptSyntaxException">Syntax error</exception>
         /// <remarks>Advances the iterator to beyond the end of the expression</remarks>
         ScriptExpressionParser::Expression*  ScriptExpressionParser::ReadBinaryExpression(TokenIterator& pos, UINT precedence)
         {
            Expression* expr = nullptr;

            // Rule: BinaryExpr = Operand (operator Operand)*
            //       Operand = Value / UnaryExpr / MultDivExpr / SumExpr / BitwiseExpr / LogicalExpr / ComparisonExpr
            try
            {
               // Read: expression-of-higher-precedence / Value
               expr = (precedence < MAX_PRECEDENCE ? ReadBinaryExpression(pos, precedence+1) : ReadUnaryExpression(pos));    // throws

               // Match: operator  
               while (MatchOperator(pos, precedence))
               {  
                  // Read: operator
                  auto op = ReadOperator(pos);                                                        

                  // Read: expression-of-higher-precedence / Value
                  auto rhs = (precedence < MAX_PRECEDENCE ? ReadBinaryExpression(pos, precedence+1) : ReadUnaryExpression(pos));  // throws
                  expr = new BinaryExpression(op, expr, rhs);     // throws
               }

               // Success:
               return expr;
            }
            catch (...)
            {  // Cleanup
               if (expr != nullptr)
                  delete expr;
               throw;
            }
         }


         /// <summary>Reads a unary expression, sub-expression, or literal</summary>
         /// <param name="pos">Position of first token of expression</param>
         /// <returns>Expression tree</returns>
         /// <exception cref="Logic::ArgumentException">Invalid precendence detected</exception>
         /// <exception cref="Logic::InvalidOperationException">Attempted to read incorrect type of Token</exception>
         /// <exception cref="Logic::ScriptSyntaxException">Syntax error</exception>
         /// <remarks>Advances the iterator to beyond the end of the expression</remarks>
         ScriptExpressionParser::Expression*  ScriptExpressionParser::ReadUnaryExpression(TokenIterator& pos)
         {
            // Rule: Unary = (! / - / ~)? Value

            // Match: Operator  
            if (MatchOperator(pos, MAX_PRECEDENCE)) 
            {   // Read: operator, Value  (may throw)
               const ScriptToken& op = ReadOperator(pos);
               return new UnaryExpression(op, ReadValue(pos));    
            }

            // Read: Value  (may throw)
            return ReadValue(pos);
         }

         /// <summary>Reads a literal or sub-expression</summary>
         /// <param name="pos">Position of literal or first token of sub-expression</param>
         /// <returns>Expression tree</returns>
         /// <exception cref="Logic::ArgumentException">Invalid precendence detected</exception>
         /// <exception cref="Logic::InvalidOperationException">Attempted to read incorrect type of Token</exception>
         /// <exception cref="Logic::ScriptSyntaxException">Syntax error</exception>
         /// <remarks>Advances the iterator to beyond the end of the literal or sub-expression</remarks>
         ScriptExpressionParser::Expression*  ScriptExpressionParser::ReadValue(TokenIterator& pos)
         {
            Expression* expr = nullptr;

            // Rule: Value = Literal / '(' Expression ')'

            // Match: Literal  
            if (MatchLiteral(pos)) 
               return new LiteralValue(ReadLiteral(pos));

            // Read: Bracket   [nothrow]
            if (!MatchOperator(pos, L"(")) 
            {
               // Failed: Unexpected EOF
               if (pos >= InputEnd)
                  throw ScriptSyntaxException(HERE, L"Missing operand");

               // Failed: Unexpected token
               throw ScriptSyntaxException(HERE, GuiString(L"Unexpected '%s'", pos->Text.c_str()));
            }
            
            // Read: Expression  (may throw)
            const ScriptToken& open = ReadOperator(pos);
            expr = ReadExpression(pos);   // Adv. then match

            // Read: Bracket   [nothrow]
            if (MatchOperator(pos, L")")) 
               return new BracketedExpression(open, expr, ReadOperator(pos));
            
            // Failure: Missing closing bracket
            delete expr;
            throw ScriptSyntaxException(HERE, L"Missing closing bracket");
         }

      }
   }
}
