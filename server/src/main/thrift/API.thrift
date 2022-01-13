//include "shared/Calibration.thrift"

namespace java org.zapo.lumosmaxima.remote.thrift


enum T_Role {
  VIEWER = 100,
  MANAGER = 200,
  ADMIN = 300,
  SYSTEM = 400
}

struct T_Login {
  1: i32 id,
  2: string login,
  3: string password,
  4: T_Role role,
  5: optional string comment,
}

struct T_FixtureManufacturer {
  1: string name
}

struct T_FixtureModels {
  1: list<T_FixtureModel> models
}

struct T_FixtureModel {
  1: string name,
  2: string manufacturer,
  3: string description,
  4: string thumbnail,
  5: list<T_FixtureMode> modes
}

struct T_FixtureMode {
  1: string name
}


service API
{
   i32 login(1: string username, 2: string password),
   void logout(),

   // Fixture Library
   list<T_FixtureManufacturer> getFixtureManufacturers(),
   T_FixtureModels getFixtureModels(1: string manufacturer),
}