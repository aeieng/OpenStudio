/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) Alliance for Sustainable Energy, LLC.
*  See also https://openstudio.net/license
***********************************************************************************************************************/

#include "../ForwardTranslator.hpp"

#include "../../model/Model.hpp"
#include "../../model/HotWaterEquipment.hpp"
#include "../../model/HotWaterEquipment_Impl.hpp"
#include "../../model/HotWaterEquipmentDefinition.hpp"
#include "../../model/HotWaterEquipmentDefinition_Impl.hpp"
#include "../../model/Space.hpp"
#include "../../model/Space_Impl.hpp"
#include "../../model/SpaceType.hpp"
#include "../../model/SpaceType_Impl.hpp"
#include "../../model/ThermalZone.hpp"
#include "../../model/ThermalZone_Impl.hpp"
#include "../../model/Schedule.hpp"
#include "../../model/Schedule_Impl.hpp"
#include "../../model/LifeCycleCost.hpp"

#include <utilities/idd/HotWaterEquipment_FieldEnums.hxx>
#include "../../utilities/idd/IddEnums.hpp"
#include <utilities/idd/IddEnums.hxx>

using namespace openstudio::model;

using namespace std;

namespace openstudio {

namespace energyplus {

  boost::optional<IdfObject> ForwardTranslator::translateHotWaterEquipment(HotWaterEquipment& modelObject) {
    IdfObject idfObject(openstudio::IddObjectType::HotWaterEquipment);
    m_idfObjects.push_back(idfObject);

    for (LifeCycleCost lifeCycleCost : modelObject.lifeCycleCosts()) {
      translateAndMapModelObject(lifeCycleCost);
    }

    HotWaterEquipmentDefinition definition = modelObject.hotWaterEquipmentDefinition();

    idfObject.setString(HotWaterEquipmentFields::Name, modelObject.name().get());

    IdfObject parentIdfObject = getSpaceLoadParent(modelObject);
    idfObject.setString(HotWaterEquipmentFields::ZoneorZoneListorSpaceorSpaceListName, parentIdfObject.nameString());

    if (boost::optional<Schedule> schedule = modelObject.schedule()) {
      auto idf_schedule_ = translateAndMapModelObject(*schedule);
      OS_ASSERT(idf_schedule_);
      idfObject.setString(HotWaterEquipmentFields::ScheduleName, idf_schedule_->nameString());
    }

    idfObject.setString(HotWaterEquipmentFields::DesignLevelCalculationMethod, definition.designLevelCalculationMethod());

    double multiplier = modelObject.multiplier();

    OptionalDouble d = definition.designLevel();
    if (d) {
      idfObject.setDouble(HotWaterEquipmentFields::DesignLevel, (*d) * multiplier);
    }

    d = definition.wattsperSpaceFloorArea();
    if (d) {
      idfObject.setDouble(HotWaterEquipmentFields::PowerperZoneFloorArea, (*d) * multiplier);
    }

    d = definition.wattsperPerson();
    if (d) {
      idfObject.setDouble(HotWaterEquipmentFields::PowerperPerson, (*d) * multiplier);
    }

    if (!definition.isFractionLatentDefaulted()) {
      idfObject.setDouble(HotWaterEquipmentFields::FractionLatent, definition.fractionLatent());
    }

    if (!definition.isFractionRadiantDefaulted()) {
      idfObject.setDouble(HotWaterEquipmentFields::FractionRadiant, definition.fractionRadiant());
    }

    if (!definition.isFractionLostDefaulted()) {
      idfObject.setDouble(HotWaterEquipmentFields::FractionLost, definition.fractionLost());
    }

    if (!modelObject.isEndUseSubcategoryDefaulted()) {
      idfObject.setString(HotWaterEquipmentFields::EndUseSubcategory, modelObject.endUseSubcategory());
    }

    return idfObject;
  }

}  // namespace energyplus

}  // namespace openstudio
