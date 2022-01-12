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

service API
{
   i32 login(1: string username, 2: string password),
   void logout(),
}