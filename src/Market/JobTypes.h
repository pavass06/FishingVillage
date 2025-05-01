#ifndef JOBTYPES_H
#define JOBTYPES_H

#include <string>
#include <memory>
class FisherMan;  // Déclaration anticipée

// Offre d’emploi
struct JobPosting {
    int firmID;
    std::string jobSector;
    int educationRequirement;
    int experienceRequirement;
    int attractiveness;
    int vacancies;
    bool recruiting;
};

// Candidature
struct JobApplication {
    int workerID;
    std::string desiredSector;
    int educationLevel;
    int experienceLevel;
    int preference;
    int quantity;
    bool matched;
    std::shared_ptr<FisherMan> fisherman;
};

#endif // JOBTYPES_H
