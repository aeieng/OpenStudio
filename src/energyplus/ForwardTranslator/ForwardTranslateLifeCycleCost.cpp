/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) Alliance for Sustainable Energy, LLC.
*  See also https://openstudio.net/license
***********************************************************************************************************************/

#include "../ForwardTranslator.hpp"

#include "../../model/Model.hpp"
#include "../../model/LifeCycleCost.hpp"
#include "../../model/LifeCycleCostParameters.hpp"
#include "../../model/LifeCycleCostParameters_Impl.hpp"

#include <utilities/idd/LifeCycleCost_NonrecurringCost_FieldEnums.hxx>
#include <utilities/idd/LifeCycleCost_RecurringCosts_FieldEnums.hxx>
#include "../../utilities/idd/IddEnums.hpp"
#include <utilities/idd/IddEnums.hxx>

using namespace openstudio::model;

using namespace std;

namespace openstudio {

namespace energyplus {

  boost::optional<IdfObject> ForwardTranslator::translateLifeCycleCost(LifeCycleCost& modelObject) {
    boost::optional<LifeCycleCostParameters> lifeCycleCostParameters = modelObject.model().lifeCycleCostParameters();
    if (!lifeCycleCostParameters) {
      lifeCycleCostParameters = modelObject.model().getUniqueModelObject<LifeCycleCostParameters>();
    }

    int lengthOfStudyPeriodInMonths = 12 * lifeCycleCostParameters->lengthOfStudyPeriodInYears();

    boost::optional<IdfObject> result;

    ModelObject item = modelObject.item();
    std::string comment = item.iddObject().name() + ", " + item.name().get();

    std::string name = modelObject.name().get();
    std::string category = modelObject.category();
    std::string startOfCosts = modelObject.startOfCosts();
    double totalCost = modelObject.totalCost();
    int yearsFromStart = modelObject.yearsFromStart();
    int monthsFromStart = modelObject.monthsFromStart();
    int repeatPeriodYears = modelObject.repeatPeriodYears();
    int repeatPeriodMonths = modelObject.repeatPeriodMonths();

    if (totalCost == 0) {
      return boost::none;
    }

    bool hasRepeatingCosts = (repeatPeriodYears > 0 || repeatPeriodMonths > 0);

    // check for field types of LifeCycleCost:NonrecurringCost {Construction, Salvage, OtherCapital}
    if (istringEqual("Construction", category) || istringEqual("Salvage", category) || istringEqual("OtherCapital", category)) {

      if (lengthOfStudyPeriodInMonths >= 12 * yearsFromStart + monthsFromStart) {

        // convert to LifeCycleCost::NonrecurringCosts
        IdfObject idfObject(openstudio::IddObjectType::LifeCycleCost_NonrecurringCost);
        idfObject.setComment(comment);
        m_idfObjects.push_back(idfObject);

        idfObject.setString(LifeCycleCost_NonrecurringCostFields::Name, name);
        idfObject.setString(LifeCycleCost_NonrecurringCostFields::Category, category);
        idfObject.setDouble(LifeCycleCost_NonrecurringCostFields::Cost, totalCost);
        idfObject.setString(LifeCycleCost_NonrecurringCostFields::StartofCosts, startOfCosts);

        if (yearsFromStart > 0) {
          idfObject.setInt(LifeCycleCost_NonrecurringCostFields::YearsfromStart, yearsFromStart);
        }

        if (monthsFromStart > 0) {
          idfObject.setInt(LifeCycleCost_NonrecurringCostFields::MonthsfromStart, monthsFromStart);
        }

        result = idfObject;

        // may create a recurring cost object to handle replacement costs, shift first replacement costs back one period
        if (istringEqual("Construction", category)) {
          name += " Replacement";
        } else {
          name += " Recurring";
        }
        category = "Replacement";
        yearsFromStart += repeatPeriodYears;
        monthsFromStart += repeatPeriodMonths;

      } else {
        //LOG(Warn, "LifeCycleCost '" << name << "' occurs outside the study period, will not be translated");
      }
    }

    if (hasRepeatingCosts) {

      if (lengthOfStudyPeriodInMonths >= 12 * yearsFromStart + monthsFromStart) {

        // also create a LifeCycleCost::RecurringCosts to capture replacement costs
        IdfObject idfObject(openstudio::IddObjectType::LifeCycleCost_RecurringCosts);
        idfObject.setComment(comment);
        m_idfObjects.push_back(idfObject);

        idfObject.setString(LifeCycleCost_RecurringCostsFields::Name, name);
        idfObject.setString(LifeCycleCost_RecurringCostsFields::Category, category);
        idfObject.setDouble(LifeCycleCost_RecurringCostsFields::Cost, totalCost);
        idfObject.setString(LifeCycleCost_RecurringCostsFields::StartofCosts, startOfCosts);

        if (yearsFromStart > 0) {
          idfObject.setInt(LifeCycleCost_RecurringCostsFields::YearsfromStart, yearsFromStart);
        }

        if (monthsFromStart > 0) {
          idfObject.setInt(LifeCycleCost_RecurringCostsFields::MonthsfromStart, monthsFromStart);
        }

        if (repeatPeriodYears > 0) {
          idfObject.setInt(LifeCycleCost_RecurringCostsFields::RepeatPeriodYears, repeatPeriodYears);
        }

        if (repeatPeriodMonths > 0) {
          idfObject.setInt(LifeCycleCost_RecurringCostsFields::RepeatPeriodMonths, repeatPeriodMonths);
        }

        if (!result) {
          result = idfObject;
        }
      } else {
        //LOG(Warn, "LifeCycleCost '" << name << "' occurs outside the study period, will not be translated");
      }
    }

    return result;
  }

}  // namespace energyplus

}  // namespace openstudio
