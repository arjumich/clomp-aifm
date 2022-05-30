//test-clomp-with-aifm-pointers

extern "C" {
#include <runtime/runtime.h>
}

#include "array.hpp"
#include "device.hpp"
#include "manager.hpp"

#include <cstdint>
#include <iostream>
#include <memory>
#include <random>

using namespace far_memory;
using namespace std;

constexpr uint64_t kCacheSize = (128ULL << 20);
constexpr uint64_t kFarMemSize = (4ULL << 30);
constexpr uint32_t kNumGCThreads = 12;
constexpr uint32_t kNumEntries =
    (16ULL << 20); // So the array size is larger than the local cache size.
constexpr uint64_t CLOMP_numParts = 1;
constexpr uint64_t CLOMP_zonesPerPart = 10;

/* Command line parameters, see usage info (initially -1 for sanity check)*/
//long CLOMP_numThreads = -2;       /* > 0 or -1 valid */
//long CLOMP_allocThreads = -2;     /* > 0 or -1 valid */
////long int CLOMP_numParts = 2;         /* > 0 valid */
////long CLOMP_zonesPerPart = -1;     /* > 0 valid */
//long CLOMP_flopScale = -1;        /* > 0 valid, 1 nominal */
//long CLOMP_timeScale = -1;        /* > 0 valid, 100 nominal */
uint64_t CLOMP_zoneSize = 100;         /* > 0 valid, (sizeof(Zone) true min)*/
//char *CLOMP_exe_name = NULL;      /* Points to argv[0] */



/* Simple Zone data structure */
struct Zone
{
    uint64_t zoneId;
    uint64_t partId;
    uint64_t value;
    struct SharedPtr<Zone> *nextZone;
};


/* Part data structure */
struct Part
{
    uint64_t partId;
    uint64_t zoneCount;
    uint64_t update_count;  
    SharedPtr<Zone> *firstZone;
    SharedPtr<Zone> *lastZone;
    uint64_t deposit_ratio;
    uint64_t residue;
    uint64_t expected_first_value; /* Used to check results */
    uint64_t expected_residue;     /* Used to check results */
};

/* LL for gdb

struct unique_ptr_part
{
  SharedPtr<Part> ptr;
  //char data[4096];
};

using  unique_ptr_part_t = struct unique_ptr_part;
*/

/* Part array working on (now array of Part pointers)*/
//Part **partArray = NULL;    //LL - What is it doing here? initializing the part double pointer 
                            // But how to modify this for aifm?


far_memory::Array<SharedPtr<Part>, CLOMP_numParts> *partArray;
//SharedPtr<Part> *partArray;
SharedPtr<Part> *pointer_loc_ptr;
SharedPtr<Zone> *pointer_loc_zone;

//**addPart is not working with AIFM. Need to check again.


void addPart (SharedPtr<Part> part, uint64_t partId)
{
  //struct Part *part_local = new Part;

  if ((partId < 0) || (partId >= CLOMP_numParts))
    {
        fprintf (stderr, "addPart error: partId (%i) out of bounds!\n", (int)partId);
        exit (1);
    }

  {
    DerefScope scope;
    auto part_val = part.deref_mut(scope);
    //part_val = part_local;


    auto &pointer_loc = partArray->at_mut(scope,partId);
    auto part_ptr = &pointer_loc;


    part_val->partId = partId;
    part_val->firstZone = nullptr;
    part_val->lastZone = nullptr;

    *pointer_loc_ptr = part;
    // auto part_ptr_val = pointer_loc_ptr->deref_mut(scope);
    // part_ptr_val = part_val;    // resembles to partArray[partId] = part

    //memset((void*)part_ptr_val->partId, static_cast<uint64_t>(partId), sizeof(uint64_t));

  //pointer_loc_ptr->write(part_local);

    // if(part_ptr_val== nullptr)
    //     {
    //       cout<< "error";
    //       //exit(0);
    //     }
    //part_val->partId = partId;

#if 0
    auto part_val_firstZone = part_val->firstZone.deref_mut(scope);
    auto part_val_lastZone = part_val->firstZone.deref_mut(scope);

    part_val->partId = partId;

    part_val_firstZone = nullptr;
    part_val_lastZone = nullptr;
#endif
    
#if 0
  //{
    //    DerefScope scope;

        auto &_pointer_loc = partArray->at_mut(scope,partId);
        auto _part_ptr = &_pointer_loc;

        cout<< "Type checking inside addPart_ Part_ptr: "<<typeid(_part_ptr).name()<<"\n";

        auto _part_ptr_val = _part_ptr->deref_mut(scope);

        cout<< "Type checking inside addPart_ Part_ptr_val: "<<typeid(_part_ptr_val).name()<<"\n";
        //cout<< "Type checking inside addPart_ Part_ptr_val: "<< static_cast<uint64_t>(_part_ptr->get_addr())<<"\n";
        
        if(_part_ptr_val== nullptr)
        {
          cout<< "error";
          //exit(0);
        }
        cout<<"partId inside addPart: " << _part_ptr_val->partId <<".\n";
#endif
  }


#if 0
  {
    
    DerefScope scope;
    auto part_val = part.deref_mut(scope);
    auto part_val_firstZone = part_val->firstZone.deref_mut(scope);
    auto part_val_lastZone = part_val->firstZone.deref_mut(scope);

    part_val->partId = partId;

    part_val_firstZone = nullptr;
    part_val_lastZone = nullptr;
    

    /* Put part pointer in array */
    //partArray.at_mut(scope,partId) = part;



    //partArray[partId] = part_val;

    /* Set partId */

  }
#endif
}

// this addZone is From CLOMP port
void addZone (SharedPtr<Part> *part, SharedPtr<Zone> & zone)
{

    if(part == nullptr)
        {
          cout<< "nullptr error inside addZone at begining! ";
          //exit(0);
        }
    //Zone *zone_local=(Zone*)malloc(sizeof(Zone*));
    struct Zone *zone_local = new Zone;

    // TODO - Sanity check(NULL check) is still remaining 
    {
        DerefScope scope;
        auto zone_val = zone.deref_mut(scope);
        cout<< "Inside addzone: type of zone_val is: "<<typeid(zone_val).name()<<".\n";
        zone_val = zone_local;

        if(&zone == nullptr)
        {
          cout<< "error";
          //exit(0);
        }


        auto part_val = part->deref_mut(scope);
        cout<< "Inside addzone: "<<typeid(part_val).name()<<".\n";
        if(part_val== nullptr)
        {
          cout<< "nullptr error inside addZone! ";
          //exit(0);
        }

        //auto part_val = part.deref_mut(scope); // is this working? 
        //auto zone_val = zone.deref_mut(scope);     // same scope for both zone and part 
        //cout<<"partId: " << part_val->partId <<".\n";

        //commented after shared pointer
        //auto part_firstzone = part_val->firstZone->deref_mut(scope);
        
        //auto part_lastzone = part_val->lastZone->deref_mut(scope);
        

        if (part_val->firstZone==nullptr)
        {
            zone_val->zoneId = 1;
            cout<< "type of part_val->lastZone: "<<typeid(part_val->lastZone).name()<<".\n";
            part_val->firstZone = &zone; 
            //
            auto part_firstzone = part_val->firstZone->deref_mut(scope);
            cout<< "zoneId of part_firstZone :"<<part_firstzone->zoneId << endl;
	        //
            part_val->lastZone = &zone;     //this probably isn't proper
            cout<< "type of part_val->lastZone: "<<typeid(part_val->lastZone).name()<<".\n";
            cout<< "type of zone: "<<typeid(zone).name()<<".\n";
        }
        else // TODO - zone dereferencing is stil incomplete; how to do for "lastZone->zoneId". Solved. Seems to be working, error gone  
        {
            /* Give this zone the last Zone's id + 1 */
            //auto lastzone_ref = part_val->lastZone.deref_mut(scope);

            auto part_lastzone = part_val->lastZone->deref_mut(scope);
            cout<< "type of part_lastzone: "<<typeid(part_lastzone).name()<<".\n";
            cout<< "zoneId of part_lastzone :"<<part_lastzone->zoneId << endl;
            zone_val->zoneId = part_lastzone->zoneId + 1;
        
            auto part_lastzone_nextZone = part_lastzone->nextZone->deref_mut(scope);
            part_lastzone->nextZone = &zone;
            part_val->lastZone = &zone;
            //zone_val->zoneId = part_val->lastZone->zoneId + 1;
            //part_val->lastZone->nextZone = zone;
        }

        //auto zone_val_nextZone = zone_val->nextZone->deref_mut(scope);
        
        zone_val->nextZone = nullptr;
        zone_val->partId = part_val->partId;
        zone_val->value = 0.0;
    }
}



#if 0

void addZone (SharedPtr<Part> part, SharedPtr<Zone> zone)
{
  //memset (zone, 0xFF, CLOMP_zoneSize);

  //Zone *zone_local=(Zone*)malloc(sizeof(Zone*));

  if(part == nullptr)
        {
          cout<< "nullptr error inside addZone at begining! ";
          //exit(0);
        }
    //Zone *zone_local=(Zone*)malloc(sizeof(Zone*));
  struct Zone *zone_local = new Zone;

  {
        DerefScope scope;
        auto zone_val = zone.deref_mut(scope);
        zone_val = zone_local;

        if(zone_val== nullptr)
        {
          cout<< "error";
          //exit(0);
        }


        auto part_val = part.deref_mut(scope);
        if(part_val== nullptr)
        {
          cout<< "nullptr error! ";
          //exit(0);
        }

        //zone_val->zoneId=var;
        //memset (zone_val->zoneId, static_cast<uint64_t>(var), CLOMP_zoneSize);  //segfault here; memset_avx2_unaligned_erms ()
  }

  //cout<<"test if it enters...........";
//#if 0
  //cout<< typeid(part).name();
  /* If not existing zones, place at head of list */
    {
      DerefScope scope;
      auto part_val = part.deref_mut(scope);
      auto zone_val = zone.deref_mut(scope);
      // if(part_val == nullptr)
      //   {
      //     cout<< "error 2";
      //     //exit(0);
      //   }

      auto part_lastzone = part_val->lastZone.deref_mut(scope);
    


      if (part_lastzone==nullptr)
      {
        zone_val->zoneId = 1;
        part_val->firstZone = std::move(zone);
	      part_val->lastZone = std::move(zone);
      }
      else 
      {
        /* Give this zone the last Zone's id + 1 */
        auto lastzone_ref = part_val->lastZone.deref_mut(scope);
        zone_val->zoneId = lastzone_ref->zoneId + 1;
    
        lastzone_ref->nextZone = std::move(zone);
        part_val->lastZone = std::move(zone);
      }

    }

}

#endif

// struct Data4096 {
//   char data[4096];
// };
// using Data_t = struct Data4096;


void do_work(FarMemManager *manager) {

//CLOMP_numThreads = convert_to_positive_long ("numThreads", argv[1]);
//CLOMP_allocThreads = convert_to_positive_long ("numThreads", argv[2]);

CLOMP_zoneSize = 32;
SharedPtr<Part> *part;  //as in clomp

//SharedPtr<Part> *pointer_loc_ptr;
//CLOMP_flopScale = convert_to_positive_long ("flopScale", argv[6]);
//CLOMP_timeScale = convert_to_positive_long ("timeScale", argv[7])

// partArray = (Part **) malloc (CLOMP_numParts * sizeof (Part*));
//     if (partArray == NULL)
//     {
// 	fprintf (stderr, "Out of memory allocating part array\n");
// 	exit (1);
//     }

//auto part_pointer = manager->allocate_unique_ptr<Part>();


//TODO need to declare this as global, error: partArray’ was not declared in this scope
//auto partArray = manager->allocate_array<SharedPtr<Part>, CLOMP_numParts>();
auto partArray1 = manager->allocate_array<SharedPtr<Part>, CLOMP_numParts>(); //last working copy
partArray = &partArray1;



//#if 0
for (uint64_t partId = 0; partId < CLOMP_numParts; partId++)
    {
      DerefScope scope;
      //partArray.at_mut(scope, partId) = NULL;
      //cout<<"Lines per part"<<endl;

      auto &pointer_loc = partArray->at_mut(scope,partId);
      pointer_loc_ptr = &pointer_loc;

      pointer_loc_ptr = nullptr;
      // auto raw_pointer_loc = pointer_loc_ptr->deref_mut(scope);
      // raw_pointer_loc = NULL;

    }

for (uint64_t partId = 0; partId < CLOMP_numParts; partId++)
    {

      auto part_ptr = manager->allocate_shared_ptr<Part>();
      //auto unq_part = manager->allocate_unique_ptr<Part>()

      //addPart(std::move(part), partId);
      addPart(std::move(part_ptr), partId);

    }

// for (uint64_t partId = 0; partId < CLOMP_numParts; partId++)
//     {
//         {
//             DerefScope scope;

//             auto &pointer_loc = partArray->at_mut(scope,partId);
//             auto part_ptr = &pointer_loc;

//             const auto part_ptr_val = part_ptr->deref(scope);
//             //cout<< "Testing partArray: "<<typeid(part_ptr_val).name()<<".\n";
//             cout<<"Debug: partId inside Parts: " << part_ptr_val->partId <<".\n";

//         }
//     }

#if 0

for (uint64_t partId = 0; partId < CLOMP_numParts; partId++)
{
	uint64_t zoneId;
  SharedPtr<Zone> zone;
  auto zone = manager->allocate_shared_ptr<Zone>();
  //far_memory::Array<SharedPtr<Zone>, CLOMP_zonesPerPart> *zoneArray;

  auto zoneArray = manager->allocate_array<SharedPtr<Zone>, CLOMP_zonesPerPart>();
  if (&zoneArray == nullptr)
    {
      fprintf (stderr, "Out of memory allocate zone array\n");
      exit (1);
    }

  for (zoneId = 0; zoneId < CLOMP_zonesPerPart; zoneId++)
	{

    /* Get the current zone being placed */
      //Zone *zone=(Zone*)malloc(sizeof(Zone*));

      SharedPtr<Zone> pointer_loc_zone_z;
      SharedPtr<Part> pointer_loc_ptr_part;
    {
        DerefScope scope;
        auto &pointer_loc_z = zoneArray.at_mut(scope,zoneId);
        pointer_loc_zone_z = &pointer_loc_z;
        //pointer_loc_zone_z = std::move(pointer_loc_z);
        zone = *pointer_loc_zone_z;

        auto &pointer_loc_part = partArray->at_mut(scope,partId);
        pointer_loc_ptr_part = &pointer_loc_part;
        //pointer_loc_ptr_part = std::move(pointer_loc_part);

        // if(pointer_loc_ptr_part == nullptr)
        // {
        //   cout<< "error inside deref addZone";
        //   //exit(0);
        // }
        
    }

    addZone (pointer_loc_ptr_part, zone);
      //addZone (std::move(pointer_loc_ptr_part), std::move(pointer_loc_zone_z)); // need to pass global here, this is the reason for segfault

  }

}

#endif
//#endif
}
void _main(void *arg) {
  std::unique_ptr<FarMemManager> manager =
      std::unique_ptr<FarMemManager>(FarMemManagerFactory::build(
          kCacheSize, kNumGCThreads, new FakeDevice(kFarMemSize)));
  do_work(manager.get());
}

int main(int argc, char *argv[]) {
  int ret;

  if (argc < 2) {
    std::cerr << "usage: [cfg_file]" << std::endl;
    return -EINVAL;
  }

  ret = runtime_init(argv[1], _main, NULL);
  if (ret) {
    std::cerr << "failed to start runtime" << std::endl;
    return ret;
  }

  return 0;
}