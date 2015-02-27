#ifndef DILAY_CONFIG
#define DILAY_CONFIG

#include "kvstore.hpp"

class Config {
  public:   
    Config (const std::string& file) : store (file, true, "config") {}

    template <class T> const T& get (const std::string& path) const {
      return this->store.get <T> (path);
    }

  private:
    KVStore store;
};

class ConfigProxy {
  public:
    ConfigProxy (Config& c, const std::string& p) 
      : config (c)
      , prefix (p) 
    {
      assert (p.back () == '/');
    }

    ConfigProxy (ConfigProxy& c, const std::string& p) 
      : ConfigProxy (c.config, c.prefix + p)
    {}

    std::string key (const std::string& p) const {
      return this->prefix + p;
    }

    template <class T> 
    const T& get (const std::string& p) const { 
      return this->config.get <T> (this->key(p));
    }

  private:
    Config&           config;
    const std::string prefix;
};

#endif
