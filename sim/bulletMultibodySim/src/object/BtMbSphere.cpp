//
// Created by kangd on 28.05.18.
//

#include "BtMbSphere.hpp"

namespace bullet_mb_sim {
namespace object {

bullet_mb_sim::object::BtMbSphere::BtMbSphere(double radius, double mass, b3RobotSimulatorClientAPI_NoGUI *api)
    : BtMbSingleBodyObject(mass, api) {

  // create collision shape
  int shapeId = -1;
  {
    b3RobotSimulatorCreateCollisionShapeArgs arg;
    arg.m_shapeType = GEOM_SPHERE;
    arg.m_radius = radius;
    shapeId = api_->createCollisionShape(GEOM_SPHERE, arg);
    RAIFATAL_IF(shapeId ==  -1, "Checkerboard shape creation error")
  }

  // create body
  {
    b3RobotSimulatorCreateMultiBodyArgs args;
    args.m_baseCollisionShapeIndex = shapeId;
    args.m_useMaximalCoordinates = 1;
    args.m_baseMass = mass;
    objectId_ = api_->createMultiBody(args);

    b3RobotSimulatorChangeDynamicsArgs dynarg;
    dynarg.m_lateralFriction = 0.8;
    dynarg.m_restitution = 0;
    dynarg.m_rollingFriction = 0;
    dynarg.m_spinningFriction = 0;
    RAIFATAL_IF(!api_->changeDynamics(objectId_, -1, dynarg), "changeDynamics failed")
  }

  RAIFATAL_IF(objectId_ ==  -1, "Checkerboard body creation error")
}

} // object
} // bullet_mb_sim


