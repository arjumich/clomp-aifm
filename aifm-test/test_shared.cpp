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
constexpr uint64_t CLOMP_numParts = 10;
constexpr uint64_t CLOMP_zonesPerPart = 10;

uint64_t CLOMP_zoneSize = 100;         /* > 0 valid, (sizeof(Zone) true min)*/




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



far_memory::Array<SharedPtr<Part>, CLOMP_numParts> *partArray;

SharedPtr<Part> *pointer_loc_ptr;


void do_work(FarMemManager *manager) {

  CLOMP_zoneSize = 32;

  auto partArray1 = manager->allocate_array<SharedPtr<Part>, CLOMP_numParts>(); //last working copy
  partArray = &partArray1;


  //#if 0
  for (uint64_t partId = 0; partId < CLOMP_numParts; partId++)
    {
      DerefScope scope;
      auto &pointer_loc = partArray->at_mut(scope,partId);
      pointer_loc_ptr = &pointer_loc;

      auto part = manager->allocate_shared_ptr<Part>();
      cout<< "Type checking inside part: " <<typeid(part).name()<<"\n";
      Part* raw_part = part.deref_mut(scope);
      raw_part->partId = partId;

      cout<< "PartId <<<<<<<<<<<<<<<<<<< " << raw_part->partId <<"\n";

      *pointer_loc_ptr = part;
    }

    for (uint64_t partId = 0; partId < CLOMP_numParts; partId++)
    {
      DerefScope scope;
      auto &pointer_loc = partArray->at_mut(scope,partId);
      auto pointer_loc_ptr1 = &pointer_loc;
      auto raw_mut_ptr = pointer_loc_ptr1->deref_mut(scope);


      cout<< "PartId >>>>>>>>>>>>>>>> " << raw_mut_ptr->partId <<"\n";
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