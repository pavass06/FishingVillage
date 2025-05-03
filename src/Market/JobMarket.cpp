#include "JobMarket.h"
#include "LabourModel.h"             
#include "FishingFirm.h"   // Pour getID(), addEmployee()
#include "FisherMan.h"     // Pour FisherMan methods

#include <iostream>
#include <algorithm>
#include <random>

// Constructeur : initialisation de tous les membres, dont params
JobMarket::JobMarket(const SimulationParameters& params,
    double initWage,
    double fishPrice,
    double meanOrder)
: Market(initWage),
  matchedJobs(0),
  meanFishOrder(meanOrder),
  currentFishPrice(fishPrice),
  firmList(nullptr),
  params(params)
{
    clearingPrice = currentFishPrice * meanFishOrder;
}


JobMarket::~JobMarket() = default;

void JobMarket::setCurrentFishPrice(double fishPrice) {
currentFishPrice = fishPrice;
}

void JobMarket::setFirmList(std::vector<std::shared_ptr<FishingFirm>>* firms) {
firmList = firms;
}

void JobMarket::submitJobPosting(const JobPosting &posting) {
postings.push_back(posting);
aggregateSupply += posting.vacancies;
}

void JobMarket::submitJobApplication(const JobApplication &application) {
JobApplication app = application;
app.matched = false;
applications.push_back(app);
aggregateDemand += app.quantity;
}

void JobMarket::clearMarket(std::default_random_engine & /*generator*/) {
// 1) Chaque firme génère ses nouvelles offres selon le modèle choisi
if (firmList) {
for (auto &firm : *firmList) {
auto posts = firm->generateJobPostings(
params.labourModel,
params.growthThreshold,
params.alpha
);
for (auto &p : posts) {
submitJobPosting(p);
}
}
}

// 2) Matching entre offres et candidatures
matchedJobs = 0;
for (auto &posting : postings) {
for (auto &app : applications) {
if (!app.matched
&& posting.jobSector == app.desiredSector
&& posting.recruiting)
{
// Embauche
posting.vacancies--;
app.matched = true;
matchedJobs++;
if (firmList) {
   for (auto &firm : *firmList) {
       if (firm->getID() == posting.firmID) {
           firm->addEmployee(app.fisherman);
           break;
       }
   }
}
if (posting.vacancies <= 0) {
   posting.recruiting = false;
   break;
}
}
}
}

// 3) Mise à jour du salaire de marché
clearingPrice = currentFishPrice * meanFishOrder;
}

void JobMarket::reset() {
postings.clear();
applications.clear();
aggregateDemand = 0;
aggregateSupply = 0;
matchedJobs = 0;
}

void JobMarket::print() const {
#if verbose==1
std::cout << "-----------" << std::endl;
std::cout << "JobMarket State:" << std::endl;
std::cout << "Aggregate Demand (Applications): " << aggregateDemand << std::endl;
std::cout << "Aggregate Supply (Vacancies): " << aggregateSupply << std::endl;
std::cout << "Clearing Wage (Based on fish price): " << clearingPrice << std::endl;
std::cout << "Matched Jobs: " << matchedJobs << std::endl;
std::cout << "Total Postings: " << postings.size()
<< " | Total Applications: " << applications.size() << std::endl;
#endif
}

double JobMarket::getClearingWage() const {
return clearingPrice;
}

int JobMarket::getMatchedJobs() const {
return matchedJobs;
}