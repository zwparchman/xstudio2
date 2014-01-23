#pragma once
#include "Common.h"
#include "XCatalog.h"
#include "XFileInfo.h"
#include "BackgroundWorker.h"

namespace Logic
{
   namespace FileSystem
   {
      /// <summary>Defines common subfolders</summary>
      enum class XFolder { Language, Scripts, Director };

      /// <summary>Contains enumerated catalogs and catalog-based files. Provides access to physical/catalog files</summary>
      class XFileSystem
      {
      private:
         /// <summary>Collection of catalogs, sorted by precedence (highest->lowest)</summary>
         class CatalogCollection : public list<XCatalog>
         {
         public:
            // --------------------- CONSTRUCTION ----------------------

            CatalogCollection() {};
            ~CatalogCollection() { clear(); };  // Unlock all catalogs

            // ---------------------- PROPERTIES -----------------------

            PROPERTY_GET(size_type,Count,size);

            // ----------------------- MUTATORS ------------------------

            void  Add(XCatalog&& c)  { push_front(std::move(c)); }
         };

         /// <summary>Collection of file descriptors</summary>
         class FileCollection : public map<Path, XFileInfo>
         {
         public:
            // ---------------------- PROPERTIES -----------------------
         
            PROPERTY_GET(size_type,Count,size);

            // ----------------------- MUTATORS ------------------------
         
            /// <summary>Attempts to add a file to the collection, overwriting any of lower precedence</summary>
            /// <param name="f">The file to add</param>
            void  Add(const XFileInfo& f)
            {
               _Pairib res = insert(value_type(f.Key, f));

               // Exists: Overwrite if higher precendence
               if (!res.second && f.Precedence > res.first->second.Precedence)
               {
                  //res.first->second = f;
                  erase(res.first);
                  insert(value_type(f.Key, f));
               }
            }
         };

      public:
         typedef list<XFileInfo>  ResultCollection;

      public:
         // --------------------- CONSTRUCTION ----------------------

         XFileSystem();
         virtual ~XFileSystem();

         // Prevent moving/copying
         NO_MOVE(XFileSystem);
         NO_COPY(XFileSystem);

         // --------------------- PROPERTIES ------------------------

         // ---------------------- ACCESSORS ------------------------

         ResultCollection Browse(XFolder  folder) const;
         ResultCollection Browse(Path  folder) const;
         bool             Contains(Path  path) const;
         XFileInfo        Find(Path  path) const;

         Path         GetFolder(XFolder f) const;
         Path         GetFolder() const            { return Folder;  }
         GameVersion  GetVersion() const           { return Version; }

			// ----------------------- MUTATORS ------------------------

      public:
         DWORD   Enumerate(Path folder, GameVersion version, WorkerData* data);
         
      private:
         DWORD   EnumerateCatalogs();
         DWORD   EnumerateFiles(WorkerData* data);
         void    EnumerateFolder(Path  folder);
         
         // -------------------- REPRESENTATION ---------------------

      private:
         CatalogCollection  Catalogs;
         FileCollection     Files;
         Path               Folder;
         GameVersion        Version;
      };

   }
}

using namespace Logic::FileSystem;
