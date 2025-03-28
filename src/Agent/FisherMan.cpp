#include "FisherMan.h"
#include "JobMarket.h"  // <- Now we need full definition here

JobApplication FisherMan::generateJobApplication() const {
    JobApplication app;
    app.workerID = getID();
    app.desiredSector = jobSector;
    app.educationLevel = educationLevel;
    app.experienceLevel = experienceLevel;
    app.preference = jobPreference;
    app.quantity = (firmID == 0) ? 1 : 0;
    app.matched = false;
    return app;
}

