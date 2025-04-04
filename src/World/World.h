#ifndef WORLD_H
#define WORLD_H

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstdlib>
#include <random>
#include <unordered_map>
#include <cmath>
#include "FisherMan.h"
#include "Firm.h"
#include "FishingFirm.h"
#include "JobMarket.h"
#include "FishingMarket.h"

class World {
private:
    int currentCycle;        // Cycle courant
    int totalCycles;         // Nombre total de cycles
    double annualBirthRate;  // Taux de naissance annuel
    int maxStarvingDays;     // Jours sans manger avant décès

    std::vector<std::shared_ptr<FisherMan>> fishers;
    std::vector<std::shared_ptr<FishingFirm>> firms;

    std::shared_ptr<JobMarket> jobMarket;
    std::shared_ptr<FishingMarket> fishingMarket;

    double GDP;
    double unemploymentRate;
    double inflation;

    double currentOfferMean;    
    double currentPerceivedMean;
    double meanAugmentationInflat;
    double varianceAugmentationInflat;
    double meanDiminutionInflat;
    double varianceDiminutionInflat;
    std::vector<double> inflations;

    std::unordered_map<int, int> daysWithoutEat;
    std::vector<double> populationAgeDistribution;

    // Historique du taux de chômage par cycle.
    std::vector<double> unemploymentHistory;

    // Paramètres pour la dynamique de l'emploi.
    double postingRate; // par ex., 0.1 (10% des employés)
    double firingRate;  // par ex., 0.05 (5% des employés)

public:
    World(int cycles,
          double annualBirthRate_,
          std::shared_ptr<JobMarket> jm,
          std::shared_ptr<FishingMarket> fm,
          int maxStarvingDays_,
          double offeredPriceMean,
          double perceivedPriceMean,
          double meanAugIn,
          double varAugIn,
          double meanDimIn,
          double varDimIn,
          double postingRate,
          double firingRate)
        : currentCycle(0),
          totalCycles(cycles),
          annualBirthRate(annualBirthRate_),
          maxStarvingDays(maxStarvingDays_),
          jobMarket(jm),
          fishingMarket(fm),
          GDP(0.0),
          unemploymentRate(0.0),
          inflation(0.0),
          currentOfferMean(offeredPriceMean),
          currentPerceivedMean(perceivedPriceMean),
          meanAugmentationInflat(meanAugIn),
          varianceAugmentationInflat(varAugIn),
          meanDiminutionInflat(meanDimIn),
          varianceDiminutionInflat(varDimIn),
          postingRate(postingRate),
          firingRate(firingRate)
    {}

    void setFirms(const std::vector<std::shared_ptr<FishingFirm>> &firmVec) {
        firms = firmVec;
    }

    void addFirm(std::shared_ptr<FishingFirm> firm) {
        firms.push_back(firm);
    }

    void addFisherMan(std::shared_ptr<FisherMan> f) {
        fishers.push_back(f);
        daysWithoutEat[f->getID()] = 0;
    }

    int getTotalFishers() const {
        return static_cast<int>(fishers.size());
    }

    double getGDP() const { return GDP; }
    double getUnemploymentRate() const { return unemploymentRate; }
    double getInflation(int day) const { return inflations[day]; }
    int getUnemployedFishers() const {
        int count = 0;
        for (const auto &fisher : fishers)
            if (fisher->getFirmID() == 0)
                count++;
        return count;
    }
    std::vector<double> getPopulationAgeDistribution() {
        populationAgeDistribution.clear();
        for (const auto &fisher : fishers) {
            populationAgeDistribution.push_back(fisher->getAge());
        }
        return populationAgeDistribution;
    }
    double getUnemployment(int day) const {
        if(day < unemploymentHistory.size())
            return unemploymentHistory[day];
        return 0.0;
    }
    const std::vector<double>& getUnemploymentHistory() const {
        return unemploymentHistory;
    }

    // Simulation d'un cycle.
    void simulateCycle(std::default_random_engine &generator,
                       std::normal_distribution<double> &firmPriceDist,
                       std::uniform_int_distribution<int> &goodsQuantityDist,
                       std::normal_distribution<double> &consumerPriceDist) {
        std::cout << "----- Début du cycle " << currentCycle + 1 << " -----" << std::endl;

        // 0) Actualiser l'offre du marché des poissons.
        if (!firms.empty()) {
            fishingMarket->refreshSupply(firms);
        } else {
            std::cerr << "Erreur : aucune firme définie dans World." << std::endl;
        }
        if (!firms.empty()) {
            double sumOffered = 0.0;
            for (const auto &firm : firms) {
                sumOffered += firm->getPriceLevel();
            }
            currentOfferMean = sumOffered / firms.size();
        }
        std::cout << "Moyenne des prix offerts : " << currentOfferMean << std::endl;

        // 1) Actions des pêcheurs.
        for (auto &fisher : fishers) {
            if (fisher->isActive())
                fisher->act();
        }
        for (auto &fisher : fishers) {
            if (fisher->isActive())
                fisher->update();
        }
        fishers.erase(std::remove_if(fishers.begin(), fishers.end(),
            [](const std::shared_ptr<FisherMan> &f) { return !f->isActive(); }),
            fishers.end());

        // 2) Gestion de la population : naissances via un processus de Poisson.
        {
            double normalizedAnnualBirthRate = (annualBirthRate > 1.0) ? annualBirthRate / 100.0 : annualBirthRate;
            double dailyBirthRate = normalizedAnnualBirthRate / 365.0;
            int currentPopulation = getTotalFishers();
            double lambda = dailyBirthRate * currentPopulation;
            std::poisson_distribution<int> poissonDist(lambda);
            int newBirths = poissonDist(generator);
            for (int i = 0; i < newBirths; i++) {
                int newID = 1000 + fishers.size();
                auto newFisher = std::make_shared<FisherMan>(
                    newID, 10.0, 365 * 60, 0.0, 0.0, 1.0, 1.0, false,
                    10.0, 0.0, "fishing", 1, 1, 1
                );
                addFisherMan(newFisher);
            }
        }

        // 3) Processus du marché de l'emploi.
        std::cout << "---- Processus du marché de l'emploi ----" << std::endl;
        
        // Sauvegarder les firmID précédents pour identifier les pêcheurs licenciés.
        std::unordered_map<int, int> prevFirmIDs;
        for (const auto &fisher : fishers) {
            prevFirmIDs[fisher->getID()] = fisher->getFirmID();
        }

        // Avant le matching, calculer la moyenne des revenus courants de toutes les firmes.
        double sumRevenue = 0.0;
        int firmCount = 0;
        for (auto &firm : firms) {
            // On utilise getRevenue() de la classe de base (qui doit être mise à jour via les ventes)
            sumRevenue += firm->getRevenue();
            firmCount++;
        }
        double avgRevenue = (firmCount > 0) ? sumRevenue / firmCount : 0.0;
        std::cout << "Moyenne des revenus des firmes ce cycle : " << avgRevenue << std::endl;

        // Firing : chaque firme licencie des employés en fonction de la comparaison avec la moyenne.
        int totalFired = 0;
        for (auto &firm : firms) {
            int beforeCount = firm->getEmployeeCount();
            firm->generateFiring(avgRevenue);
            int afterCount = firm->getEmployeeCount();
            totalFired += (beforeCount - afterCount);
        }

        // Posting : chaque firme génère des offres d'emploi en fonction de la comparaison avec la moyenne.
        int totalPostings = 0;
        for (auto &firm : firms) {
            std::vector<JobPosting> firmPostings = firm->generateJobPostings(avgRevenue, "fishing", 1, 1, 1);
            totalPostings += firmPostings.size();
            for (const auto &posting : firmPostings) {
                jobMarket->submitJobPosting(posting);
            }
        }

        // Les pêcheurs sans emploi (firmID == 0) et cherchant activement un travail postulent.
        int applicationsCount = 0;
        for (auto &fisher : fishers) {
            if (fisher->getFirmID() == 0 && fisher->isLookingForJob()) {
                JobApplication app = fisher->generateJobApplication();
                app.fisherman = fisher;
                jobMarket->submitJobApplication(app);
                applicationsCount++;
            }
        }

        std::cout << " agg demand [l232] ===> " << jobMarket->getAggregateDemand() << std::endl;
        std::cout << " applicationsCount " << applicationsCount << std::endl;
        
        

        std::cout << "Nombre total de candidatures soumises : " << applicationsCount << std::endl;


        std::cout << " aggregateSupply ===> " << jobMarket->getAggregateSupply() << std::endl;


        // Matching : le JobMarket effectue le matching et embauche via addEmployee().
        jobMarket->clearMarket(generator);
        int matches = jobMarket->getMatchedJobs();
        std::cout << "Nombre de correspondances réalisées : " << matches << std::endl;
        std::cout << "Nombre de personnes embauchées ce cycle : " << matches << std::endl;
        jobMarket->reset();

        // Debug : identification de l'état d'emploi actuel.
        std::vector<int> unemployedIDs;
        std::vector<int> employedIDs;
        std::vector<int> lookingIDs;
        std::vector<int> firedIDs;
        for (const auto &fisher : fishers) {
            int id = fisher->getID();
            if (fisher->getFirmID() == 0) {
                unemployedIDs.push_back(id);
                if (fisher->isLookingForJob())
                    lookingIDs.push_back(id);
            } else {
                employedIDs.push_back(id);
            }
            if (prevFirmIDs[id] != 0 && fisher->getFirmID() == 0) {
                firedIDs.push_back(id);
            }
        }

        std::cout << "---- Détails du marché de l'emploi ----" << std::endl;
        std::cout << "FISHERS EN RECHERCHE D'EMPLOI (Looking for job): ";
        for (int id : lookingIDs)
            std::cout << id << " ";
        std::cout << std::endl;
        std::cout << "FISHERS EMPLOYÉS (Having a job): ";
        for (int id : employedIDs)
            std::cout << id << " ";
        std::cout << std::endl;
        std::cout << "FISHERS LICENCIÉS CE CYCLE (Fired this cycle): ";
        for (int id : firedIDs)
            std::cout << id << " ";
        std::cout << std::endl;
        std::cout << "Récapitulatif:" << std::endl;
        std::cout << "  - Nombre total de licenciements : " << totalFired << std::endl;
        std::cout << "  - Nombre total d'offres d'emploi postées : " << totalPostings << std::endl;
        std::cout << "  - Nombre total de pêcheurs au chômage : " << unemployedIDs.size() << std::endl;
        std::cout << "  - Nombre de correspondances (embauches) réalisées : " << matches << std::endl;

        // Enregistrer les revenus de chaque firme pour ce cycle.
        for (auto &firm : firms) {
            firm->firmRevenue(currentCycle, firms.size());
        }

        // 4) Commandes sur le marché des poissons.
        double sumPerceived = 0.0;
        int orderCount = 0;
        for (auto &fisher : fishers) {
            FishOrder order;
            order.id = fisher->getID();
            order.desiredSector = "fishing";
            order.quantity = 1.0;
            order.perceivedValue = consumerPriceDist(generator);
            sumPerceived += order.perceivedValue;
            orderCount++;
            order.availableFunds = fisher->getFunds();
            order.hungry = (daysWithoutEat[fisher->getID()] > 0);
            fishingMarket->submitFishOrder(order);
        }
        currentPerceivedMean = (orderCount > 0) ? sumPerceived / orderCount : 0.0;
        std::cout << "Nombre de commandes de poissons soumises : " << orderCount << std::endl;

        fishingMarket->setAggregateDemand(static_cast<double>(getTotalFishers()));
        fishingMarket->clearMarket(generator);

        // 5) Calcul du GDP.
        double dailyGDP = 0.0;
        int nfirm=0; double sumrevenue=0.0;
        int totalfishsold=0;
        for (auto &firm : firms) {

            double aux = firm->getRevenue(); 
            dailyGDP += aux;
            firm->resetSales(); ////<----------------
            std::cout << " firm : " << nfirm << " revenue=  " << aux ;
            double fishsold = firm->getSales();
            std::cout << " fish sold : " << fishsold ;
            double revenueperfish = aux/std::max(fishsold,1.0);
            std::cout << " revenue per fish : " << revenueperfish  ;
            int nemployees= firm-> getEmployeeCount();

            std::cout << " employees : " << nemployees << std::endl;


            totalfishsold+=fishsold;  
            nfirm++;
            sumrevenue += aux;
         
        }

        sumrevenue /= nfirm;
        std::cout << "Average revenue : " << sumrevenue  << std::endl;


        

        GDP = dailyGDP;
        std::cout << "GDP quotidien : " << dailyGDP << std::endl;


        

        // 6) Calcul du taux de chômage.
        int unemployedCount = 0;
        for (const auto &fisher : fishers)
            if (fisher->getFirmID() == 0)
                unemployedCount++;
        unemploymentRate = (fishers.size() > 0) ? static_cast<double>(unemployedCount) / fishers.size() : 0.0;
        std::cout << "Taux de chômage : " << unemploymentRate * 100 << "%" << std::endl;
        unemploymentHistory.push_back(unemploymentRate);

        // 7) Famine: mise à jour des jours sans manger.
        std::unordered_map<int, double> purchases = fishingMarket->getPurchases();
        for (auto &fisher : fishers) {
            int fID = fisher->getID();
            if (purchases.find(fID) == purchases.end() || purchases[fID] < 1.0)
                daysWithoutEat[fID]++;
            else
                daysWithoutEat[fID] = 0;
        }
        for (auto &fisher : fishers) {
            int fID = fisher->getID();
            if (daysWithoutEat[fID] >= maxStarvingDays) {
                fisher->setActive(false);
            }
        }
        fishers.erase(std::remove_if(fishers.begin(), fishers.end(),
            [](const std::shared_ptr<FisherMan>& f) { return !f->isActive(); }),
            fishers.end());
        std::cout << "Nombre de pêcheurs après famine : " << fishers.size() << std::endl;

        // 8) Calcul de l'inflation.
        double prevClearingPrice = 0.0, newClearingPrice = 0.0;
        const std::vector<double>& priceHistory = fishingMarket->getClearingPriceHistory();
        if (priceHistory.size() >= 2) {
            prevClearingPrice = priceHistory[priceHistory.size() - 2];
            newClearingPrice = priceHistory.back();
            double inflRate = (prevClearingPrice > 0) ? (newClearingPrice - prevClearingPrice) / prevClearingPrice : 0.0;
            inflation = inflRate;
            inflations.push_back(inflation);
        } else {
            inflation = 0.0;
            inflations.push_back(0.0);
        }
        std::cout << "Inflation : " << inflation * 100 << "%" << std::endl;

        jobMarket->print();

        currentCycle++;
        std::cout << "----- Fin du cycle " << currentCycle << " -----" << std::endl << std::endl;
    }

    void printWorldState() const {
        std::cout << "=== World State at Day " << currentCycle << " ===" << std::endl;
        std::cout << "Population: " << getTotalFishers() << std::endl;
        std::cout << "GDP: " << GDP << std::endl;
        std::cout << "Unemployment Rate: " << unemploymentRate * 100 << "%" << std::endl;
        std::cout << "Inflation: " << inflation * 100 << "%" << std::endl;
        std::cout << "Job Market:" << std::endl;
        jobMarket->print();
        std::cout << "Fishing Market:" << std::endl;
        fishingMarket->print();
    }
};

#endif // WORLD_H
