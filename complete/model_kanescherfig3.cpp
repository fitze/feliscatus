
#include "FelisCatusModeling.h"

using OpenSim::CoordinateCouplerConstraint;
using OpenSim::LinearFunction;
using OpenSim::PinJoint;
using OpenSim::PointToPointSpring;

/**
 * Creates an OpenSim model of a cat, sufficient for exploring the cat's
 * righting reflex.
 *
 * This model allows for the "salient features of motion" of a falling cat
 * first identified by Kane and Scher (1969). These include:
 * 	1) The torso bends but does not twist.
 * 	2) At release, the spine is bent forward. Afterwards, the spine is
 * 	   first bent to one side, the backward, then to the other side,
 * 	   and finally forwards again. At the end of this "oscillation,"
 * 	   the cat has turned over.
 * 	3) The backwards bend that the cat experiences during its turning
 * 	   maneuver is more pronounced that its initial or final forward
 * 	   bends.
 * These salient features are implemented starting with Figure 2 of their paper.
 *
 * NOTE: In its current state, this code does not correctly implement Kane and
 * Scher's model.
 * */
class KaneScherFig3Modeling : public FelisCatusModeling
{
    void addBaseJointsAndBodies();
    void addBaseForcesAndActuation();
};

/** Creates a FelisCatus model with a spinal joint defined by Kane and Scher,
 * with the topology based off Figure 3 of Kane and Scher (1969).
 * */
int main(int argc, char *argv[])
{
    KaneScherFig3Modeling m;
    m.makeModel("Leland_kanescherfig3");
    m.printModel("feliscatus_kanescherfig3.osim");

    return EXIT_SUCCESS;
}

/**
 * The topology with this joint is:
 * 
 *                   Aintermed -> anteriorBody
 *                 /
 * ground -> frameQ 
 *                 \
 *                   Bintermed -> posteriorBody
 * 
 * */
void KaneScherFig3Modeling::addBaseJointsAndBodies()
{
    Body & ground = _cat.getGroundBody();

    // I'm calling massless bodies "frames". Here are all the ones I'm
    // using:
    // The frame containing A1 and B1.
    Body * frameQ = newMasslessBody("Q");
    // Frame rotated from Q about N by -gamma/2, to which anteriorBody is
    // attached.
    Body * Aintermed = newMasslessBody("Aintermed");
    // Frame rotated from Q about N by gamma/2, to which posteriorBody is
    // attached.
    Body * Bintermed = newMasslessBody("Bintermed");

    // -- Connect Q to ground.
    Vec3 locGQInGround(0);
    // TODO need some description here.
    // Y rotation to place the joint axis in the ground's X-Y plane.
    Vec3 orientGQInGround(0, 0.5 * Pi, 0);
    // TODO X rotation to offset the fixed location of the joint axis in space,
    // so that it is inclined from the ground X axis, toward the ground Y axis.
    // TODO Vec3 orientGQInGround(0.25 * Pi, 0.5 * Pi, 0);
    Vec3 locGQInFrameQ(0);
    // Y rotation to undo previous rotation on other side of joint so that
    // cat lies in X-Y plane.
    // Z rotation to set the default configuration of the cat to be upside
    // down.
    Vec3 orientGQInFrameQ(0, -0.5 * Pi, Pi);
    PinJoint * groundFrameQ = new PinJoint("ground_frameQ",
            ground, locGQInGround, orientGQInGround,
            *frameQ, locGQInFrameQ, orientGQInFrameQ);
    CoordinateSet & groundFrameQCS = groundFrameQ->upd_CoordinateSet();
    groundFrameQCS[0].setName("kane_psi");
    double groundFrameQCS0range[2] = {-1.5 * Pi, 1.5 * Pi};
    groundFrameQCS[0].setRange(groundFrameQCS0range);
    groundFrameQCS[0].setDefaultValue(0.0);
    groundFrameQCS[0].setDefaultLocked(false);

    // Connect Aintermed(iate) to Q.
    Vec3 locQAIinFrameQ(0);
    Vec3 orientQAIinFrameQ(0, 0, 0);
    Vec3 locQAIinAintermed(0);
    Vec3 orientQAIinAintermed(0, 0, 0);
    PinJoint * frameQAintermed = new PinJoint("frameQ_Aintermed",
            *frameQ, locQAIinFrameQ, orientQAIinFrameQ,
            *Aintermed, locQAIinAintermed, orientQAIinAintermed);
    CoordinateSet & frameQAintermedCS =
        frameQAintermed->upd_CoordinateSet();
    frameQAintermedCS[0].setName("kane_gammaA");
    double frameQAintermedCS0range[2] = {-0.5 * Pi, 0.25 * Pi};
    frameQAintermedCS[0].setRange(frameQAintermedCS0range);
    frameQAintermedCS[0].setDefaultValue(0.0);
    frameQAintermedCS[0].setDefaultLocked(false);

    // Connect Bintermed(iate) to Q.
    Vec3 locQBIinFrameQ(0);
    Vec3 orientQBIinFrameQ(0, 0, 0);
    Vec3 locQBIinBintermed(0);
    Vec3 orientQBIinBintermed(0, 0, 0);
    PinJoint * frameQBintermed = new PinJoint("frameQ_Bintermed",
            *frameQ, locQBIinFrameQ, orientQBIinFrameQ,
            *Bintermed, locQBIinBintermed, orientQBIinBintermed);
    CoordinateSet & frameQBintermedCS =
        frameQBintermed->upd_CoordinateSet();
    frameQBintermedCS[0].setName("kane_gammaB");
    double frameQBintermedCS0range[2] = {0, 0.5 * Pi};
    frameQBintermedCS[0].setRange(frameQBintermedCS0range);
    frameQBintermedCS[0].setDefaultValue(0.0);
    frameQBintermedCS[0].setDefaultLocked(false);

    // Connect Aintermed to anteriorBody.
    Vec3 locAIAinAintermed(0);
    Vec3 orientAIAinAintermed(0, 0.5 * Pi, 0);
    Vec3 locAIAinAnterior(0);
    Vec3 orientAIAinAnterior(0, -0.5 * Pi, 0);
    PinJoint * AintermedAnterior = new PinJoint("Aintermed_anterior",
            *Aintermed, locAIAinAintermed, orientAIAinAintermed,
            *_anteriorBody, locAIAinAnterior, orientAIAinAnterior);
    CoordinateSet & AintermedAnteriorCS =
        AintermedAnterior->upd_CoordinateSet();
    // "integ" because this coord is the integral of the speed u.
    AintermedAnteriorCS[0].setName("kane_u_integ");
    double AintermedAnteriorCS0range[2] = {-2 * Pi, 2 * Pi};
    AintermedAnteriorCS[0].setRange(AintermedAnteriorCS0range);
    AintermedAnteriorCS[0].setDefaultValue(0.0);
    AintermedAnteriorCS[0].setDefaultLocked(false);

    // Connect Bintermed to posteriorBody.
    Vec3 locBIBinBintermed(0);
    Vec3 orientBIBinBintermed(0, 0.5 * Pi, 0);
    Vec3 locBIBinPosterior(0);
    Vec3 orientBIBinPosterior(0, -0.5 * Pi, 0);
    PinJoint * BintermedPosterior = new PinJoint("Bintermed_posterior",
            *Bintermed, locBIBinBintermed, orientBIBinBintermed,
            *_posteriorBody, locBIBinPosterior, orientBIBinPosterior);
    CoordinateSet & BintermedPosteriorCS =
        BintermedPosterior->upd_CoordinateSet();
    BintermedPosteriorCS[0].setName("kane_v_integ");
    double BintermedPosteriorCS0range[2] = {-2 * Pi, 2 * Pi};
    BintermedPosteriorCS[0].setRange(BintermedPosteriorCS0range);
    BintermedPosteriorCS[0].setDefaultValue(0.0);
    BintermedPosteriorCS[0].setDefaultLocked(false);


    // --- Display geometry for the time being to help see relation to Fig 3.
    // This geometry represents the plane/frame Q.
    DisplayGeometry * frameQDisplay = new DisplayGeometry("box.vtp");
    frameQDisplay->setOpacity(0.2);
    frameQDisplay->setScaleFactors(Vec3(2, 2, 0.01));

    frameQ->updDisplayer()->updGeometrySet().adoptAndAppend(frameQDisplay);
    frameQ->updDisplayer()->setShowAxes(true);

    // --- Add bodies.
    // Need to do this before the constraints.
    _cat.addBody(frameQ);
    _cat.addBody(Aintermed);
    _cat.addBody(Bintermed);
    _cat.addBody(_anteriorBody);
    _cat.addBody(_posteriorBody);

    // --- Constraints.
    // -- Constraint between intermediate frames.
    CoordinateCouplerConstraint * intermedConstr = 
        new CoordinateCouplerConstraint();
    intermedConstr->setName("intermed");
    intermedConstr->setIndependentCoordinateNames(
            Array<string>("kane_gammaA", 1));
    intermedConstr->setDependentCoordinateName("kane_gammaB");
    Array<double> intermedConstrFcnCoeff;
    intermedConstrFcnCoeff.append(-1);
    intermedConstr->setFunction(new LinearFunction(intermedConstrFcnCoeff));

    // -- Constraint between kane_u_integ and kane_v_integ.
    CoordinateCouplerConstraint * twistConstr = 
        new CoordinateCouplerConstraint();
    twistConstr->setName("twist");
    twistConstr->setIndependentCoordinateNames(
            Array<string>("kane_u_integ", 1));
    twistConstr->setDependentCoordinateName("kane_v_integ");
    Array<double> twistConstrFcnCoeff;
    twistConstrFcnCoeff.append(1);
    twistConstr->setFunction(new LinearFunction(twistConstrFcnCoeff));

    // -- Add constraints to model.
    _cat.addConstraint(intermedConstr);
    _cat.addConstraint(twistConstr);
    
}

void KaneScherFig3Modeling::addBaseForcesAndActuation()
{
    // This actuator is here to play around with the idea of using linear
    // actuators to actuate Kane and Scher's model. It is not actually part of
    // the Kane and Scher model itself.

    string body1Name = "anteriorBody";
    Vec3 point1(-0.5 * _segmentalLength, 0, 0.5 * _segmentalDiam);
    string body2Name = "posteriorBody";
    Vec3 point2(0.5 * _segmentalLength, 0, 0.5 * _segmentalDiam);
    double stiffness = 1.0;
    double restlength = 0.75 * _segmentalLength;
    PointToPointSpring * act1 = new PointToPointSpring(body1Name, point1,
                                                       body2Name, point2,
                                                       stiffness, restlength);
    act1->setName("test_spin");
    _cat.addForce(act1);
}
