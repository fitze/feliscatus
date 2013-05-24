
#include "FelisCatusModeling.h"

/**
 * This creates a "tree" of cat models, starting with a "zero-
 * degree-of-freedom" (DOF) base case and adding complexity (i.e.,
 * DOFs) one step at a time. Each cat model consists of two cylindrical
 * segments connected by a three-DOF (back flexion or 'hunch', spinal
 * twist, and side bending or 'wag') joint. Each model restricts the
 * DOFs of this joint to a different degree. In addition, some models
 * includes legs; some are rigid, normal to the cat's underside, while
 * others are "retractable," allowing the cat to change the effective
 * moment of inertia of each of its halves.
 * */

class FelisCatusModel : public FelisCatusModeling
{
public:

	enum LegsType {None, Rigid, RetractBack, Retract};

    // member functions to allow for stepwise model creation
	// NOTE: each 'add' function takes care of actuators if necessary
	FelisCatusModel(string modelName, string fileName,
		bool canTwist, bool canHunch, bool canWag, LegsType whichLegs);
	void createBaseCase();
    void addTwist();
	void addHunch();
	void addWag();
	void addRigidLegs();
	void addRetractLegs(bool frontLegsRetract=true);

private:

	static const int maxTorque = 100; // N-m
};

int main(int argc, char *argv[])
{
    vector<bool> bothOptions;
    bothOptions.push_back(false);
    bothOptions.push_back(true);

    vector<bool> canTwist = bothOptions;
    vector<bool> canHunch = bothOptions;
    vector<bool> canWag = bothOptions;

    vector<string> twistStrings;

    vector<FelisCatusModel::LegsType> whichLegs;
    whichLegs.push_back(FelisCatusModel::Rigid);
    whichLegs.push_back(FelisCatusModel::RetractBack);
    whichLegs.push_back(FelisCatusModel::Retract);

    for (unsigned int it = 0; it < canTwist.size(); it++)
    {
        for (unsigned int ih = 0; ih < canHunch.size(); ih++)
        {
            for (unsigned int iw = 0; iw < canWag.size(); iw++)
            {
                for (int iL = 0; iL < whichLegs.size(); iL++)
                {
                    string modifier = "_";
                    modifier += canTwist[it] ? "Twist" : "";
                    modifier += canHunch[ih] ? "Hunch" : "";
                    modifier += canWag[iw] ? "Wag" : "";

                    switch (whichLegs[iL])
                    {
                        case FelisCatusModel::Rigid:
                            modifier += "RigidLegs"; break;
                        case FelisCatusModel::RetractBack:
                            modifier += "RetractBackLeg"; break;
                        case FelisCatusModel::Retract:
                            modifier += "RetractLegs"; break;
                    }
                    FelisCatusModel("Leland" + modifier,
                            "feliscatus" + modifier + ".osim",
                            canTwist[it], canHunch[ih], canWag[iw],
                            whichLegs[iL]);
                }
            }
        }
    }

    return EXIT_SUCCESS;
};

FelisCatusModel::FelisCatusModel(string modelName, string fileName,
		bool canTwist, bool canHunch, bool canWag, LegsType whichLegs)
{
	makeModel(modelName, fileName);
	
	createBaseCase();
	
	if (canTwist) {
		addTwist();
	}

	if (canHunch) {
		addHunch();
	}

	if (canWag) {
		addWag();
	}

	if (whichLegs == Rigid) {
		addRigidLegs();
    } else if (whichLegs == RetractBack) {
        addRetractLegs(false);
	} else if (whichLegs == Retract) {
		addRetractLegs();
	}

	cat.print(fileName);
}

void FelisCatusModel::createBaseCase()
{
    Body & ground = cat.getGroundBody();

    // Connecting the anterior body to the ground via a custom joint. 
	// Rotation is defined via YZX Euler angles, named yaw, pitch, and
	// roll respectively. The important translation is in Y, the direction
	// of gravity.	
	Vec3 locGAInGround(0);
    Vec3 orientGAInGround(0);
    Vec3 locGAInAnterior(0);
    Vec3 orientGAInAnterior(0);

	SpatialTransform groundAnteriorST;
	groundAnteriorST.updTransformAxis(0).setCoordinateNames(
            Array<string>("yaw", 1));
    groundAnteriorST.updTransformAxis(0).setAxis(Vec3(0, 1, 0));
    groundAnteriorST.updTransformAxis(1).setCoordinateNames(
            Array<string>("pitch", 1));
    groundAnteriorST.updTransformAxis(1).setAxis(Vec3(0, 0, 1));
    groundAnteriorST.updTransformAxis(2).setCoordinateNames(
            Array<string>("roll", 1));
    groundAnteriorST.updTransformAxis(2).setAxis(Vec3(1, 0, 0));
    groundAnteriorST.updTransformAxis(3).setCoordinateNames(
            Array<string>("tx", 1));
    groundAnteriorST.updTransformAxis(3).setAxis(Vec3(1, 0, 0));
    groundAnteriorST.updTransformAxis(4).setCoordinateNames(
            Array<string>("ty", 1));
    groundAnteriorST.updTransformAxis(4).setAxis(Vec3(0, 1, 0));
    groundAnteriorST.updTransformAxis(5).setCoordinateNames(
            Array<string>("tz", 1));
    groundAnteriorST.updTransformAxis(5).setAxis(Vec3(0, 0, 1));

    CustomJoint * groundAnterior = new CustomJoint("ground_anterior",
            ground, locGAInGround, orientGAInGround,
            *anteriorBody, locGAInAnterior, orientGAInAnterior,
            groundAnteriorST);

    CoordinateSet & groundAnteriorCS = groundAnterior->upd_CoordinateSet();
    // yaw
    double groundAnteriorCS0range[2] = {-Pi, Pi};
    groundAnteriorCS[0].setRange(groundAnteriorCS0range);
    groundAnteriorCS[0].setDefaultValue(0);
    groundAnteriorCS[0].setDefaultLocked(false);
    // pitch
    double groundAnteriorCS1range[2] = {-Pi, Pi};
    groundAnteriorCS[1].setRange(groundAnteriorCS1range);
    groundAnteriorCS[1].setDefaultValue(0);
    groundAnteriorCS[1].setDefaultLocked(false);
    // roll
    double groundAnteriorCS2range[2] = {-Pi, Pi};
    groundAnteriorCS[2].setRange(groundAnteriorCS2range);
    groundAnteriorCS[2].setDefaultValue(0);
    groundAnteriorCS[2].setDefaultLocked(false);
    // tx
    double groundAnteriorCS3range[2] = {-1, 1};
    groundAnteriorCS[3].setRange(groundAnteriorCS3range);
    groundAnteriorCS[3].setDefaultValue(0);
	groundAnteriorCS[3].setDefaultLocked(true);
    // ty
    double groundAnteriorCS4range[2] = {-1, 5};
    groundAnteriorCS[4].setRange(groundAnteriorCS4range);
    groundAnteriorCS[4].setDefaultValue(2);
	groundAnteriorCS[4].setDefaultLocked(false);
    // tz
    double groundAnteriorCS5range[2] = {-1, 1};
    groundAnteriorCS[5].setRange(groundAnteriorCS5range);
    groundAnteriorCS[5].setDefaultValue(0);
    groundAnteriorCS[5].setDefaultLocked(true);

    // Connecting the anterior and posterior bodies via a custom joint. 
	// Rotation is defined via ZYX Euler angles, named hunch, wag, and
	// twist respectively.
    Vec3 locAPInAnterior(0);
    Vec3 orientAPInAnterior(0);
    Vec3 locAPInPosterior(0);
    Vec3 orientAPInPosterior(0);

	SpatialTransform anteriorPosteriorST;
	anteriorPosteriorST.updTransformAxis(0).setCoordinateNames(
            Array<string>("hunch", 1));
    anteriorPosteriorST.updTransformAxis(0).setAxis(Vec3(0, 0, 1));
    anteriorPosteriorST.updTransformAxis(1).setCoordinateNames(
            Array<string>("wag", 1));
    anteriorPosteriorST.updTransformAxis(1).setAxis(Vec3(0, 1, 0));
    anteriorPosteriorST.updTransformAxis(2).setCoordinateNames(
            Array<string>("twist", 1));
	anteriorPosteriorST.updTransformAxis(2).setAxis(Vec3(1, 0, 0));

    CustomJoint * anteriorPosterior = new CustomJoint("anterior_posterior",
            *anteriorBody, locAPInAnterior, orientAPInAnterior,
            *posteriorBody, locAPInPosterior, orientAPInPosterior,
			anteriorPosteriorST);

    CoordinateSet & anteriorPosteriorCS = anteriorPosterior->upd_CoordinateSet();
    // hunch
    double anteriorPosteriorCS0range[2] = {-0.5 * Pi, 0.5 * Pi};
    anteriorPosteriorCS[0].setRange(anteriorPosteriorCS0range);
    anteriorPosteriorCS[0].setDefaultValue(0);
    anteriorPosteriorCS[0].setDefaultLocked(true);
	// wag
    double anteriorPosteriorCS1range[2] = {-0.5 * Pi, 0.5 * Pi};
    anteriorPosteriorCS[1].setRange(anteriorPosteriorCS1range);
    anteriorPosteriorCS[1].setDefaultValue(0);
    anteriorPosteriorCS[1].setDefaultLocked(true);
	// twist
    double anteriorPosteriorCS2range[2] = {-0.5 * Pi, 0.5 * Pi};
    anteriorPosteriorCS[2].setRange(anteriorPosteriorCS2range);
    anteriorPosteriorCS[2].setDefaultValue(0);
    anteriorPosteriorCS[2].setDefaultLocked(true);

    cat.addBody(anteriorBody);
    cat.addBody(posteriorBody);
}

void FelisCatusModel::addTwist()
{
	// Unlock twist.
	cat.updCoordinateSet().get("twist").setDefaultLocked(false);
	
	// Add twist actuator.
	CoordinateActuator * twistAct = new CoordinateActuator("twist");
    twistAct->setName("twist_actuator");
	twistAct->setMinControl(-maxTorque);
    twistAct->setMaxControl(maxTorque);
	cat.addForce(twistAct);
}

void FelisCatusModel::addHunch()
{
	// Unlock hunch.
	cat.updCoordinateSet().get("hunch").setDefaultLocked(false);
	
	// Add hunch actuator.
	CoordinateActuator * hunchAct = new CoordinateActuator("hunch");
	hunchAct->setName("hunch_actuator");
	hunchAct->setMinControl(-maxTorque);
    hunchAct->setMaxControl(maxTorque);
	cat.addForce(hunchAct);
}

void FelisCatusModel::addWag()
{
	// Unlock wag.
	cat.updCoordinateSet().get("wag").setDefaultLocked(false);
	
	// Add wag actuator.
	CoordinateActuator * wagAct = new CoordinateActuator("wag");
	wagAct->setName("wag_actuator");
	wagAct->setMinControl(-maxTorque);
    wagAct->setMaxControl(maxTorque);
	cat.addForce(wagAct);
}

void FelisCatusModel::addRigidLegs()
{
	Vec3 locALegsInAnterior(-0.75 * segmentalLength, 0.5 * segmentalDiam, 0);
    Vec3 orientALegsInAnterior(0);
    Vec3 locALegsInLegs(0);
    Vec3 orientALegsInLegs(0, 0, -0.5 * Pi);
    WeldJoint * anteriorToLegs = new WeldJoint("anterior_legs",
            *anteriorBody, locALegsInAnterior, orientALegsInAnterior,
            *anteriorLegs, locALegsInLegs, orientALegsInLegs);

	Vec3 locPLegsInPosterior(0.75 * segmentalLength, 0.5 * segmentalDiam, 0);
    Vec3 orientPLegsInPosterior(0, Pi, 0);
    Vec3 locPLegsInLegs(0);
    Vec3 orientPLegsInLegs(0, 0, -0.5 * Pi);
    WeldJoint * posteriorToLegs = new WeldJoint("posterior_legs",
            *posteriorBody, locPLegsInPosterior, orientPLegsInPosterior,
            *posteriorLegs, locPLegsInLegs, orientPLegsInLegs);

	cat.addBody(anteriorLegs);
	cat.addBody(posteriorLegs);
}

void FelisCatusModel::addRetractLegs(bool frontLegsRetract)
{
	// Adding 1-DOF legs.
	Vec3 locALegsInAnterior(-0.75 * segmentalLength, 0.5 * segmentalDiam, 0);
    Vec3 orientALegsInAnterior(0);
    Vec3 locALegsInLegs(0);
    Vec3 orientALegsInLegs(0, 0, -0.5 * Pi);
    PinJoint * anteriorToLegs = new PinJoint("anterior_legs",
            *anteriorBody, locALegsInAnterior, orientALegsInAnterior,
            *anteriorLegs, locALegsInLegs, orientALegsInLegs);
    CoordinateSet & anteriorToLegsCS = anteriorToLegs->upd_CoordinateSet();
    anteriorToLegsCS[0].setName("frontLegs");
    double anteriorToLegsCS0range[2] = {-0.5 * Pi, 0.5 * Pi};
    anteriorToLegsCS[0].setRange(anteriorToLegsCS0range);
    if (frontLegsRetract)
    {
        anteriorToLegsCS[0].setDefaultValue(0);
        anteriorToLegsCS[0].setDefaultLocked(false);
    }
    else
    {
        anteriorToLegsCS[0].setDefaultValue(0.25 * Pi);
        anteriorToLegsCS[0].setDefaultLocked(true);
    }

	Vec3 locPLegsInPosterior(0.75 * segmentalLength, 0.5 * segmentalDiam, 0);
    Vec3 orientPLegsInPosterior(0, Pi, 0);
    Vec3 locPLegsInLegs(0);
    Vec3 orientPLegsInLegs(0, 0, -0.5 * Pi);
    PinJoint * posteriorToLegs = new PinJoint("posterior_legs",
            *posteriorBody, locPLegsInPosterior, orientPLegsInPosterior,
            *posteriorLegs, locPLegsInLegs, orientPLegsInLegs);
    CoordinateSet & posteriorToLegsCS = posteriorToLegs->upd_CoordinateSet();
    posteriorToLegsCS[0].setName("backLegs");
    double posteriorToLegsCS0range[2] = {-0.5 * Pi, 0.5 * Pi};
    posteriorToLegsCS[0].setRange(posteriorToLegsCS0range);
    posteriorToLegsCS[0].setDefaultValue(0);
    posteriorToLegsCS[0].setDefaultLocked(false);

	cat.addBody(anteriorLegs);
	cat.addBody(posteriorLegs);

	// Adding leg actuators.
    if (frontLegsRetract)
    {
        CoordinateActuator * frontLegsAct = new CoordinateActuator("frontLegs");
        frontLegsAct->setName("frontLegs_actuator");
        frontLegsAct->setMinControl(-maxTorque);
        frontLegsAct->setMaxControl(maxTorque);
        cat.addForce(frontLegsAct);
    }
    
	CoordinateActuator * backLegsAct = new CoordinateActuator("backLegs");
    backLegsAct->setName("backLegs_actuator");
    backLegsAct->setMinControl(-maxTorque);
    backLegsAct->setMaxControl(maxTorque);
    cat.addForce(backLegsAct);
}