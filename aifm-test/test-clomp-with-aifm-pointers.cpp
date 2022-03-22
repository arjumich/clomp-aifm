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

/* Command line parameters, see usage info (initially -1 for sanity check)*/
//long CLOMP_numThreads = -2;       /* > 0 or -1 valid */
//long CLOMP_allocThreads = -2;     /* > 0 or -1 valid */
////long int CLOMP_numParts = 2;         /* > 0 valid */
////long CLOMP_zonesPerPart = -1;     /* > 0 valid */
//long CLOMP_flopScale = -1;        /* > 0 valid, 1 nominal */
//long CLOMP_timeScale = -1;        /* > 0 valid, 100 nominal */
long CLOMP_zoneSize = -1;         /* > 0 valid, (sizeof(Zone) true min)*/
//char *CLOMP_exe_name = NULL;      /* Points to argv[0] */


/* Simple Zone data structure */
typedef struct _Zone
{
    long zoneId;
    long partId;
    double value;
    struct _Zone *nextZone;
} Zone;


/* Part data structure */
typedef struct _Part
{
    long partId;
    long zoneCount;
    long update_count;  
    Zone *firstZone;
    Zone *lastZone;
    double deposit_ratio;
    double residue;
    double expected_first_value; /* Used to check results */
    double expected_residue;     /* Used to check results */
} Part;

/* Part array working on (now array of Part pointers)*/
Part **partArray = NULL;    //LL - What is it doing here? initializing the part double pointer 
                            // But how to modify this for aifm?


//**addPart is not working with AIFM. Need to check again.
void addPart (Part *part, long partId)
{
  DerefScope scope;

  /* Put part pointer in array */
  partArray.at_mut(scope,partId) = part;
  //partArray[partId] = part;

  /* Set partId */
  part->partId = partId;
    
  /* Set zone count for part (now fixed, used to be variable */
  //part->zoneCount = CLOMP_zonesPerPart;
}

void addZone (Part **part, Zone *zone)
{
  memset (zone, 0xFF, CLOMP_zoneSize);

  /* If not existing zones, place at head of list */
    if ((*part)->lastZone == NULL)
    {
	    /* Give first zone a zoneId of 1 */
	    zone->zoneId = 1;

	    /* First and last zone */
	    (*part)->firstZone = zone;
	    (*part)->lastZone = zone;

    }

    else 
    {
	    /* Give this zone the last Zone's id + 1 */
	    zone->zoneId = (*part)->lastZone->zoneId + 1;
	
	    (*part)->lastZone->nextZone = zone;
	    (*part)->lastZone = zone;
    }

    /* Always placed at end */
    zone->nextZone = NULL;
    
    /* Inialized the rest of the zone fields */
    zone->partId = (*part)->partId;
    zone->value = 0.0;
}

void do_work(FarMemManager *manager) {

//CLOMP_numThreads = convert_to_positive_long ("numThreads", argv[1]);
//CLOMP_allocThreads = convert_to_positive_long ("numThreads", argv[2]);
constexpr long CLOMP_numParts = 2;
constexpr long CLOMP_zonesPerPart = 10;
CLOMP_zoneSize = 32;
//CLOMP_flopScale = convert_to_positive_long ("flopScale", argv[6]);
//CLOMP_timeScale = convert_to_positive_long ("timeScale", argv[7])

// partArray = (Part **) malloc (CLOMP_numParts * sizeof (Part*));
//     if (partArray == NULL)
//     {
// 	fprintf (stderr, "Out of memory allocating part array\n");
// 	exit (1);
//     }

auto part_pointer = manager->allocate_unique_ptr<Part>();

auto partArray = manager->allocate_array<part_pointer, CLOMP_numParts>();
for (long partId = 0; partId < CLOMP_numParts; partId++)
    {
      DerefScope scope;
      partArray.at_mut(scope, partId) = NULL;
      cout<<"Lines per part"<<endl;
    } 

for (long partId = 0; partId < CLOMP_numParts; partId++)
    {
	    Part* part;
	    // if ((part= (Part *) malloc (sizeof (Part))) == NULL)
	    //   {
	    //     fprintf (stderr, "Out of memory allocating part\n");
	    //     exit (1);
	    //   }
      //uncomment when activating addPart function body
      addPart(part, partId);
    }

for (long partId = 0; partId < CLOMP_numParts; partId++)
{
	//Zone *zoneArray, *zone;
  //Zone *zone;
	int zoneId;

  auto zoneArray = manager->allocate_array<Zone *, CLOMP_zonesPerPart>();

  for (zoneId = 0; zoneId < CLOMP_zonesPerPart; zoneId++)
	{
    /* Get the current zone being placed */
    DerefScope scope;
	  auto zone = &zoneArray.at(scope,zoneId);
	    
	  /* Add it to the end of the the part */
	  addZone (partArray.at_mut(scope,partId), *zone);

  }

}

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
