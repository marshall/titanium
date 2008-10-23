#ifdef USING_CCTESTS

#include "gears/localserver/common/localserver.h"
#include "gears/localserver/common/managed_resource_store.h"
#include "gears/localserver/common/resource_store.h"
#include "gears/base/common/permissions_db.h"
#include "gears/base/common/security_model.h"
#include "gears/base/common/stopwatch.h"

static const char16 *kEmptyString = STRING16(L"");
static const char16 *kXUnderbar = STRING16(L"x_");

// This is the function we export. It runs the perf tests and returns
// the results in string form.
bool RunLocalServerPerfTests(int num_origins, int num_stores, int num_items,
                             std::string16 *results);


//------------------------------------------------------------------------------
// Utils to form origins and names of stores and urls of items that don't
// collide. 
//------------------------------------------------------------------------------

static bool ItemHasAdditions(int i) {
  // Used to jumble the string so sort order doesn't match iteration order
  // and to append query strings to a subset of the urls we use.
  return (i % 2) != 0;
}

static void MaybeAddToItemUrl(int i, std::string16 *url,
                              const char16 *addition) {
  if (ItemHasAdditions(i)) {
    (*url) += addition;
  }
}

static bool GetPerfTestOrigin(int i, SecurityOrigin *origin) {
  std::string16 url(STRING16(L"http://cc_perf_tests_"));
  MaybeAddToItemUrl(i, &url, kXUnderbar);
  url += IntegerToString16(i);
  return origin->InitFromUrl(url.c_str());
}

static std::string16 GetStoreName(int i) {
  std::string16 name(STRING16(L"store_"));
  name += IntegerToString16(i);
  return name;
}

static std::string16 GetManagedStoreName(int i) {
  std::string16 name(STRING16(L"managed_store_"));
  name += IntegerToString16(i);
  return name;
}

static std::string16 GetItemUrlWithPath(const LocalServer *store, int i,
                                        const char16 *item_path) {
  std::string16 url(store->GetSecurityOrigin().url());
  url += STRING16(L"/");
  url += store->GetName();
  url += item_path;
  MaybeAddToItemUrl(i, &url, kXUnderbar);
  url += IntegerToString16(i);
  MaybeAddToItemUrl(i, &url, STRING16(L"?query=true"));
  return url;

}

static std::string16 GetItemUrl(const LocalServer *store, int i) {
  return GetItemUrlWithPath(store, i, STRING16(L"/item_"));
}

static std::string16 GetNearMissUrl(const LocalServer *store, int i) {
  return GetItemUrlWithPath(store, i, STRING16(L"/miss_"));
}

static std::string16 GetFarMissUrl(int i) {
  std::string16 url(STRING16(L"http://cc_perf_test_noperms_"));
  url += STRING16(L"/far_miss_");
  MaybeAddToItemUrl(i, &url, kXUnderbar);
  url += IntegerToString16(i);
  MaybeAddToItemUrl(i, &url, STRING16(L"?query=true"));
  return url;
 }
 

//------------------------------------------------------------------------------
// Crude struct to tally up samples and a scoped helper to add samples
//------------------------------------------------------------------------------

struct PerfStats {
  int num_samples;
  int total_time;
  int max_time;
  int min_time;
  int avg_time() const { return num_samples ? total_time/num_samples : 0; }

  PerfStats(): num_samples(0), total_time(0),
               max_time(kint32min), min_time(kint32max) {}

  void AddSample(int time) {
    ++num_samples;
    total_time += time;
    max_time = std::max(max_time, time);
    min_time = std::min(min_time, time);
  }

  void AppendResults(std::string16 *results) const {
    if (!num_samples) {
      *results += STRING16(L"no samples\n");
      return;
    }
    *results += STRING16(L"n = ");
    *results += IntegerToString16(num_samples);
    *results += STRING16(L", avg = ");
    *results += IntegerToString16(avg_time());
    *results += STRING16(L", min = ");
    *results += IntegerToString16(min_time);
    *results += STRING16(L", max = ");
    *results += IntegerToString16(max_time);
    *results += STRING16(L", sum = ");
    *results += IntegerToString16(total_time);
    *results += STRING16(L"\n");
  }
};

struct ScopedPerfSampler {
  ScopedPerfSampler(PerfStats *stats) : perf_stats(stats) {
    stopwatch.Start();
  }

  ~ScopedPerfSampler() {
    stopwatch.Stop();
    perf_stats->AddSample(stopwatch.GetElapsed());
  }

  PerfStats *perf_stats;
  Stopwatch stopwatch;
};


//------------------------------------------------------------------------------
// The perf test class, encapsulates the test code and storage for the
// resulting collection of PerfStats.
//
// These tests grant permissions to many origins which are populated with
// test data, and then undoes all of that. Time samples are taken during the 
// buildup and teardown process.
//------------------------------------------------------------------------------

class LocalServerPerfTests {
 public:
  bool Run(int num_origins, int num_stores, int num_items);

  // The resulting PerfStats
  PerfStats service_hits_stats_;
  PerfStats service_hits_q_stats_;
  PerfStats service_near_misses_stats_;
  PerfStats service_near_misses_q_stats_;
  PerfStats service_far_misses_stats_;
  PerfStats service_far_misses_q_stats_;
  PerfStats total_stats_;
  PerfStats allow_stats_;
  PerfStats disallow_stats_;
  PerfStats create_store_stats_;
  PerfStats create_managed_stats_;
  PerfStats open_store_stats_;
  PerfStats open_managed_stats_;
  PerfStats remove_full_store_stats_;
  PerfStats remove_empty_store_stats_;
  PerfStats remove_managed_stats_;
  PerfStats insert_item_stats_;
  PerfStats batch_copy_stats_;
  PerfStats remove_item_stats_;

  void AppendResults(std::string16 *results);

 private:
  // The following determine how large a data set is built up
  int num_origins_;
  int num_stores_per_origin_;
  int num_items_per_store_;
 
  bool RunManyOrigins();
  void DisallowManyOrigins();

  bool AllowOrigin(const SecurityOrigin &origin);
  bool DisallowOrigin(const SecurityOrigin &origin);

  bool PopulateAndAccessOrigin(const SecurityOrigin &origin);
  bool DepopulateOrigin(const SecurityOrigin &origin);

  bool PopulateStore(ResourceStore *store);
  bool AccessStore(const ResourceStore *store);
  bool DepopulateStore(ResourceStore *store);
};

static bool RunOnce(int num_origins, int num_stores, int num_items,
                    std::string16 *results) {
  LocalServerPerfTests tests;
  bool ok = tests.Run(num_origins, num_stores, num_items);
  if (ok) {
    tests.AppendResults(results);
  } else {
    *results += STRING16(L"ERROR - LocalServerPerfTests failed\n");
  }
  return ok;
}

// Our exported function
bool RunLocalServerPerfTests(int num_origins, int num_stores, int num_items,
                             std::string16 *results) {
  results->clear();

  WebCacheDB *db = WebCacheDB::GetDB();
  if (!db) return false;

  *results += STRING16(L"// Origins = ");
  *results += IntegerToString16(num_origins);
  *results += STRING16(L", Stores = ");
  *results += IntegerToString16(num_stores);
  *results += STRING16(L", Items = ");
  *results += IntegerToString16(num_items);

  // Run without indexes
  *results += STRING16(L"\n\n// Without indexes\n");
  if (!db->DropIndexes()) {
    *results += STRING16(L"ERROR - Failed to drop indexes!!!\n");
    return false;
  }
  bool ok = RunOnce(num_origins, num_stores, num_items, results);
  if (!db->CreateIndexes()) {  // Recreate before proceeding
    *results = STRING16(L"ERROR - Failed to recreate indexes!!!\n");
    return false;
  }
  if (!ok) {
    return false;
  }

  // Run with indexes
  *results += STRING16(L"\n// With indexes\n");
  ok = RunOnce(num_origins, num_stores, num_items, results);

  return ok;
}

void LocalServerPerfTests::AppendResults(std::string16 *results) {
  static const struct {
      const char16 *label;
      const PerfStats *stats;
  } stats[] =  
    {
      { STRING16(L"service_hits:          "), &service_hits_stats_ },
      { STRING16(L"service_hits_q:        "), &service_hits_q_stats_ },
      { STRING16(L"service_near_misses:   "), &service_near_misses_stats_ },
      { STRING16(L"service_near_misses_q: "),  &service_near_misses_q_stats_ },
      { STRING16(L"service_far_misses:    "), &service_far_misses_stats_ },
      { STRING16(L"service_far_misses_q:  "), &service_far_misses_q_stats_ },
      { STRING16(L"allow_perms:         "), &allow_stats_ },
      { STRING16(L"disallow_perms:      "), &disallow_stats_ },
      { STRING16(L"create_store:        "), &create_store_stats_ },
      { STRING16(L"create_managed:      "), &create_managed_stats_ },
      { STRING16(L"open_store:          "), &open_store_stats_ },
      { STRING16(L"open_managed:        "), &open_managed_stats_ },
      { STRING16(L"remove_empty_store:  "), &remove_empty_store_stats_ },
      { STRING16(L"remove_full_store:   "), &remove_full_store_stats_ },
      { STRING16(L"remove_managed:      "), &remove_managed_stats_ },
      { STRING16(L"batch_copy:          "), &batch_copy_stats_},
      { STRING16(L"insert_item:         "), &insert_item_stats_ },
      { STRING16(L"remove_item:         "), &remove_item_stats_ },
      { STRING16(L"total:               "), &total_stats_ },
    };

  for (size_t i = 0; i < ARRAYSIZE(stats); ++i) {
    *results += stats[i].label;
    stats[i].stats->AppendResults(results);
  }
}

bool LocalServerPerfTests::Run(int num_origins, int num_stores, int num_items) {
  num_origins_ = num_origins;
  num_stores_per_origin_ = num_stores;
  num_items_per_store_ = num_items;

  ScopedPerfSampler total_sampler(&total_stats_);
  bool ok = RunManyOrigins();
  if (!ok)
    DisallowManyOrigins();
  return ok;
}


bool LocalServerPerfTests::RunManyOrigins() {
  for (int i = 0; i < num_origins_; ++i) {
    SecurityOrigin origin;
    if (!GetPerfTestOrigin(i, &origin) ||
        !AllowOrigin(origin) ||
        !PopulateAndAccessOrigin(origin)) {
      return false;
    }
  }

  for (int i = 0; i < num_origins_; ++i) {
    SecurityOrigin origin;
    if (!GetPerfTestOrigin(i, &origin) ||
        !DepopulateOrigin(origin) ||
        !DisallowOrigin(origin)) {
      return false;
    }
  }
  return true;
}

void LocalServerPerfTests::DisallowManyOrigins() {
  for (int i = 0; i < num_origins_; ++i) {
    SecurityOrigin origin;
    if (GetPerfTestOrigin(i, &origin)) {
      DisallowOrigin(origin);
    }
  }
}


bool LocalServerPerfTests::AllowOrigin(const SecurityOrigin &origin) {
  PermissionsDB *permissions = PermissionsDB::GetDB();
  if (!permissions) return false;

  ScopedPerfSampler allow_sampler(&allow_stats_);
  permissions->SetPermission(origin,
                             PermissionsDB::PERMISSION_LOCAL_DATA,
                             PermissionsDB::PERMISSION_ALLOWED);
  return true;
}

bool LocalServerPerfTests::DisallowOrigin(const SecurityOrigin &origin) {
  PermissionsDB *permissions = PermissionsDB::GetDB();
  if (!permissions) return false;

  ScopedPerfSampler disallow_sampler(&disallow_stats_);
  permissions->SetPermission(origin,
                             PermissionsDB::PERMISSION_LOCAL_DATA,
                             PermissionsDB::PERMISSION_NOT_SET);
  return true;
}


bool LocalServerPerfTests::PopulateAndAccessOrigin(
                               const SecurityOrigin &origin) {
  for (int i = 0; i < num_stores_per_origin_; ++i) {
    ResourceStore store;
    { 
      ScopedPerfSampler sampler(&create_store_stats_);
      if (!store.CreateOrOpen(origin, GetStoreName(i).c_str(), kEmptyString))
        return false;
    }

    if (!PopulateStore(&store))
      return false;

    if (!AccessStore(&store))
      return false;
  }

  for (int i = 0; i < num_stores_per_origin_; ++i) {
    ManagedResourceStore store;
    { 
      ScopedPerfSampler sampler(&create_managed_stats_);
      if (!store.CreateOrOpen(origin, GetManagedStoreName(i).c_str(),
                              kEmptyString)) {
        return false;
      }
    }

    // TODO(michaeln): populate the managed store and access urls from within it
  }

  return true;
}

bool LocalServerPerfTests::PopulateStore(ResourceStore *store) {
  // Individually insert one to get timing samples
  std::string16 first_url = GetItemUrl(store, 0);
  {
    const char *data = "Hello world";
    const char16 *headers =
        STRING16(L"Content-Type: text/plain\r\nContent-Length: 11\r\n\r\n");

    ResourceStore::Item item;
    item.entry.url = first_url;
    item.payload.headers = headers;
    item.payload.data.reset(new std::vector<uint8>);
    item.payload.data->assign(data, data + strlen(data));
    item.payload.status_line = STRING16(L"HTTP/1.0 200 OK");
    item.payload.status_code = HttpConstants::HTTP_OK;

    ScopedPerfSampler sampler(&insert_item_stats_);
    if (!store->PutItem(&item))
      return false;
  }
  
  // Batch insert the rest so we can build up a large data set faster by
  // copying the first item into many urls.
  ScopedPerfSampler sampler(&batch_copy_stats_);
  SQLTransaction transaction(WebCacheDB::GetDB()->GetSQLDatabase(),
                             "PerfTest.batch_copy");
  if (!transaction.Begin()) {
    return false;
  }

  for (int i = 1; i < num_items_per_store_; ++i) {
    std::string16 url = GetItemUrl(store, i);
    if (!store->Copy(first_url.c_str(), url.c_str()))
      return false;
  }
  
  return transaction.Commit();
}


bool LocalServerPerfTests::AccessStore(const ResourceStore *store) {
  WebCacheDB *db = WebCacheDB::GetDB();

  // Access urls in the database
  for (int i = 0; i < num_items_per_store_; ++i) {
    scoped_ptr<ScopedPerfSampler> sampler;
    if (!ItemHasAdditions(i))
      sampler.reset(new ScopedPerfSampler(&service_hits_stats_));
    else
      sampler.reset(new ScopedPerfSampler(&service_hits_q_stats_));
    WebCacheDB::PayloadInfo payload;
    if (!db->Service(GetItemUrl(store, i).c_str(), NULL, true, &payload))
      return false;
  }

  // Access urls that are not in the database, but are within origins that
  // have permission. We query the localserver database in this case. 
  for (int i = 0; i < num_items_per_store_; ++i) {
    scoped_ptr<ScopedPerfSampler> sampler;
    if (!ItemHasAdditions(i))
      sampler.reset(new ScopedPerfSampler(&service_near_misses_stats_));
    else
      sampler.reset(new ScopedPerfSampler(&service_near_misses_q_stats_));
    WebCacheDB::PayloadInfo payload;
    if (db->Service(GetNearMissUrl(store, i).c_str(), NULL, true, &payload))
      return false;  // we expect this to not be serviceable
  }

  // Access urls that are not in the database, and are not within origins that
  // have permission. We should not hit the localserver db in this case.
  for (int i = 0; i < num_items_per_store_; ++i) {
    scoped_ptr<ScopedPerfSampler> sampler;
    if (!ItemHasAdditions(i))
      sampler.reset(new ScopedPerfSampler(&service_far_misses_stats_));
    else
      sampler.reset(new ScopedPerfSampler(&service_far_misses_q_stats_));
    WebCacheDB::PayloadInfo payload;
    if (db->Service(GetFarMissUrl(i).c_str(), NULL, true, &payload))
      return false;  // we expect this to not be serviceable
  }

  return true;
}


bool LocalServerPerfTests::DepopulateOrigin(const SecurityOrigin &origin) {
  for (int i = 0; i < num_stores_per_origin_; ++i) {
    ResourceStore store;
    { 
      int64 id;
      ScopedPerfSampler sampler(&open_store_stats_);
      if (!ResourceStore::ExistsInDB(origin, GetStoreName(i).c_str(),
                                     kEmptyString, &id)) {
        return false;
      }
      if (!store.Open(id))
        return false;
    }

    // Every other store, skip removing indiviual items prior to removing
    // the store so we gather stats distinctly on those two cases.
    if (i % 2) {
     if (!DepopulateStore(&store))
       return false;
      ScopedPerfSampler sampler(&remove_empty_store_stats_);
      if (!store.Remove())
        return false;
    } else {
      ScopedPerfSampler sampler(&remove_full_store_stats_);
      if (!store.Remove())
        return false;
    }
  }

  for (int i = 0; i < num_stores_per_origin_; ++i) {
    ManagedResourceStore store;
    { 
      int64 id;
      ScopedPerfSampler sampler(&open_managed_stats_);
      if (!ManagedResourceStore::ExistsInDB(origin,
                                            GetManagedStoreName(i).c_str(),
                                            kEmptyString, &id)) {
        return false;
      }
      if (!store.Open(id))
        return false;
    }
    { 
      ScopedPerfSampler sampler(&remove_managed_stats_);
      if (!store.Remove())
        return false;
    }
  }
  return true;
}

bool LocalServerPerfTests::DepopulateStore(ResourceStore *store) {
  for (int i = 0; i < num_items_per_store_; ++i) {
    std::string16 url = GetItemUrl(store, i);
    ScopedPerfSampler remove_item_sampler(&remove_item_stats_);
    if (!store->Delete(url.c_str()))
      return false;
  }
  return true;
}

#endif  // USING_CCTESTS
