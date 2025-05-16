#ifndef WORLD_H
#define WORLD_H
#define verbose 1

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstdlib>
#include <random>
#include <unordered_map>
#include <cmath>
#include "../helper.h"
#include "../SimulationParameters.h"
#include "FisherMan.h"
#include "Firm.h"
#include "FishingFirm.h"
#include "JobMarket.h"
#include "FishingMarket.h"
#include "LabourModel.h"


class World {
    private:
        // **NOUVEAU** : référence aux paramètres globaux
        SimulationParameters params;
    
        int currentCycle;        // Cycle courant
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
        std::vector<double> unemploymentHistory;
    
        double postingRate; 
        double firingRate;  

        int nsalespercycle;       // per-cycle sales of the *current* firm
        int nsalestotal;          // cumulative sales of the *current* firm
        int cycleSalesAllFirms;   // per-cycle sales across *all* firms
        int totalSalesAllFirms;   // cumulative sales across *all* firms

    public:

    // constructor
    World(SimulationParameters params_in,
        std::shared_ptr<JobMarket> jm,
        std::shared_ptr<FishingMarket> fm)
      : params(params_in),
        currentCycle(0),
        annualBirthRate(params_in.annualBirthRate),
        maxStarvingDays(params_in.maxStarvingDays),
        fishers(),
        firms(),
        jobMarket(jm),
        fishingMarket(fm),
        GDP(0.0),
        unemploymentRate(0.0),
        inflation(0.0),
        currentOfferMean(params_in.offeredPriceMean),
        currentPerceivedMean(params_in.perceivedPriceMean),
        meanAugmentationInflat(params_in.meanAugmentationInflat),
        varianceAugmentationInflat(params_in.varianceAugmentationInflat),
        meanDiminutionInflat(params_in.meanDiminutionInflat),
        varianceDiminutionInflat(params_in.varianceDiminutionInflat),
        postingRate(params_in.postingRate),
        firingRate(params_in.firingRate),
        nsalespercycle(0),
        nsalestotal(0),
        cycleSalesAllFirms(0),
        totalSalesAllFirms(0)
    {}
    //
  
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
    double getInflation(int day) const {
        if (day >= 0 && static_cast<std::size_t>(day) < inflations.size()) {
            return inflations[static_cast<std::size_t>(day)];
        }
        return 0.0;
    }

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
        if (day >= 0 && static_cast<std::size_t>(day) < unemploymentHistory.size()) {
            return unemploymentHistory[static_cast<std::size_t>(day)];
        }
        return 0.0;
    }
    
    void print_vars()
    {
        std::cout << " --------------------------------------- " << std::endl;
        std::cout << " variables in World " << std::endl; 
        std::cout << " GDP: " << GDP << std::endl;
        std::cout << " unemploymentRate: " << unemploymentRate << std::endl;
        std::cout << " inflation:" << inflation << std::endl;
        std::cout << " currentOfferMean:" << currentOfferMean << std::endl;
        std::cout << " currentPerceivedMean:" << currentPerceivedMean << std::endl;
        std::cout << " meanAugmentationInflat:" << meanAugmentationInflat << std::endl;
        std::cout << " varianceAugmentationInflat:" << varianceAugmentationInflat << std::endl;
        std::cout << " meanDiminutionInflat:" << meanDiminutionInflat << std::endl;
        std::cout << " varianceDiminutionInflat:" << varianceDiminutionInflat << std::endl;
        std::cout << " postingRate:" << postingRate << std::endl;
        std::cout << " firingRate:" <<firingRate << std::endl; 
        std::cout << " --------------------------------------- " << std::endl;
    }


    const std::vector<double>& getUnemploymentHistory() const {
        return unemploymentHistory;
    }
    
    // Simulation d'un cycle.
    void simulateCycle(std::default_random_engine &generator,
                       std::normal_distribution<double> /* &firmPriceDist */,
                       std::uniform_int_distribution<int> /* &goodsQuantityDist */,
                       std::normal_distribution<double> &consumerPriceDist) {
        nsalespercycle     = 0;
        cycleSalesAllFirms = 0;
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
        Print("Moyenne des prix offerts",currentOfferMean);
    
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
            double dailyBirthRate = normalizedAnnualBirthRate / days_in_year;
            int currentPopulation = getTotalFishers();
            double lambda = dailyBirthRate * currentPopulation;
            std::poisson_distribution<int> poissonDist(lambda);
            int newBirths = poissonDist(generator);
            int nFishers= static_cast<int>(fishers.size());
            int newID = fishers.size() + nFishers; 
            for (int i = 0; i < newBirths; i++) {
                newID++;
                auto newFisher = std::make_shared<FisherMan>(
                    newID, 0.0, days_in_year * 60, 0.0, 0.0, 1.0, 1.0, false,
                    10.0, 0.0, "fishing", 1, 1, 1
                );
                addFisherMan(newFisher);
            }
        }
    
        // 3) Processus du marché de l'emploi.
        std::cout << "---- Processus du marché de l'emploi ----" << std::endl;

        // Save previous firmID for tracking fired fishermen.
        std::unordered_map<int, int> prevFirmIDs;
        for (const auto &fisher : fishers) {
            prevFirmIDs[fisher->getID()] = fisher->getFirmID();
        }

        // ------------------------
        // Compute revenue statistics for all firms
        std::vector<double> firmRevenues;
        for (auto &firm : firms) {
            double firmRev = 0.0;
            if (!firm->getRevenueHistory().empty()) {
                firmRev = firm->getRevenueHistory().back();
            } else {
                firmRev = firm->getRevenue(); // For the first cycle
            }
            firmRevenues.push_back(firmRev);
        }
        
         // Debug: Identify current employment state.
         std::vector<int> unemployedIDs;
         for (const auto &fisher : fishers) {
             if (fisher->getFirmID() == 0) unemployedIDs.push_back(fisher->getID());
         }
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
 
        // Hiring or Firing based on selected labour model
        std::unordered_map<int, std::pair<int,int>> firmFlows;
        int totalHires = 0;
        int totalFires = 0;
        unemployedIDs.clear();
        cycleSalesAllFirms = 0;

        for (auto& firm : firms) {
            int firmID = firm->getID();
            int delta  = 0;
            firmFlows.clear();
            unemployedIDs.clear();
            int cycleSalesAllFirms = 0;
            int sumRev = 0.0;

            // compare strings, since SimulationParameters::labourModel is a string
            if      (params.labourModel == "PastPerformance") {
                delta = LabourDemandModels::computeJobsPastPerformance(
                            *firm, params.growthThreshold);
            }
            else if (params.labourModel == "EUBI") {
                delta = LabourDemandModels::computeJobsEUBI(
                            *firm, params.alpha);
            }
            int hires = 0;
            int fires = 0;
            if (delta > 0) {
                hires = delta;
                totalHires += hires;
                auto postings = firm->generateJobPostings(
                                    params.labourModel,
                                    params.growthThreshold,
                                    params.alpha);
                for (auto& p : postings)
                    jobMarket->submitJobPosting(p);
            }
            else if (delta < 0) {
                fires = -delta;
                totalFires += fires;
                firm->fireEmployees(fires);
                // actually remove ’fires’ fishermen from this firm
                int fired = 0;
                for (auto &fisher : fishers) {
                    if (fisher->getFirmID() == firmID && fired < fires) {
                        firm->removeEmployee(fisher);
                        ++fired;
                    }
                }
            }
            firmFlows[firmID] = {hires, fires};
        }
        
        for (auto &fisher : fishers) {
            if (fisher->isLookingForJob()) {
                auto app = fisher->generateJobApplication();
                app.fisherman = fisher;              // link back to the shared_ptr
                jobMarket->submitJobApplication(app);
            }
        }

        jobMarket->clearMarket(generator);
        int matches = jobMarket->getMatchedJobs();
        jobMarket->reset();
        

        // If a firm has no employees, it is removed from the simulation.
        firms.erase(std::remove_if(firms.begin(), firms.end(),
        [](const std::shared_ptr<FishingFirm>& firm) {
            return firm->getEmployeeCount() == 0;
        }), firms.end());

        // now your verbose debug:
        #if verbose
        std::cout << "---- Details du marché de l'emploi ----" << std::endl;
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
        std::cout << "---- Employment Market Recap ----\n";
        for (auto &e : firmFlows)
            std::cout << "Firm " << e.first
                    << " hired " << e.second.first
                    << ", fired " << e.second.second << "\n";
        std::cout << "Total hired this cycle: " << totalHires << "\n"
                << "Total fired this cycle: " << totalFires << "\n"
                << "Unemployed count: "        << unemployedIDs.size() << "\n"
                << "Unemployment rate (%): "   << unemploymentRate*100 << "%\n";
        #endif


        // --- before you open the loop ---
        int orderCount = 0;
        double sumPerceived = 0.0;
        double minAskPrice = fishingMarket->getMinAskPrice();

        for (auto &fisher : fishers) {
            double funds = fisher->getFunds();
            if (funds < minAskPrice) 
                continue;           // skip those with no buying power

            FishOrder order;
            order.id             = fisher->getID();
            order.desiredSector  = "fishing";
            order.quantity       = 1.0;
            order.perceivedValue = consumerPriceDist(generator);
            order.availableFunds = funds;
            order.hungry         = (daysWithoutEat[fisher->getID()] > 0);

            fishingMarket->submitFishOrder(order);
            sumPerceived += order.perceivedValue;
            ++orderCount;
        }

        // **Key change**: use actual orders, not total fishers
        fishingMarket->setAggregateDemand(static_cast<double>(orderCount));
        currentPerceivedMean = (orderCount > 0) ? sumPerceived / orderCount : 0.0;
        Print("Nombre de commandes de poissons soumises", orderCount);

        fishingMarket->clearMarket(generator);

        // Now, record the revenue for each firm only once—after processing fish market orders.
        for (auto &firm : firms) {
            firm->recordRevenue();
        }
    
        // 5) Calcul du GDP.
        

        // 6) GDP & sales debug
        double dailyGDP=0, sumRev=0;
        int nfirm=0;
        for (auto& f: firms) {
            double rev = f->getRevenue();
            dailyGDP += rev;

            int soldThisCycle = f->getSales();
            int soldTotal     = f->getCumulativeSales();
            f->resetSales();

            // per‐firm counters
            nsalespercycle = soldThisCycle;
            nsalestotal    = soldTotal;
            // all‐firms per‐cycle
            cycleSalesAllFirms += soldThisCycle;

            double revPerFish = soldThisCycle>0? rev/soldThisCycle:0.0;
            int emp = f->getEmployeeCount();
            ++nfirm;
            sumRev += rev;
#if verbose
printf(
    "firm %2d | sold:%5d | rev/fish:%7.4f | emp:%3d | thisFirm_cycle:%5d | thisFirm_total:%7d\n",
    nfirm, soldThisCycle, revPerFish, emp, nsalespercycle, nsalestotal
  );
#endif
}
sumRev /= nfirm;
GDP = dailyGDP;

#if verbose
        totalSalesAllFirms += cycleSalesAllFirms;
        std::cout << "Average revenue : " << sumRev << "\n"
                  << "GDP quotidien   : " << dailyGDP << "\n"
                  << "All‐firms this cycle: " << cycleSalesAllFirms
                  << " | All‐firms total: "     << totalSalesAllFirms
                  << "\n";
#endif
    
        // 6) Calcul du taux de chômage.
        int unemployedCount = 0;
        for (const auto &fisher : fishers)
            if (fisher->getFirmID() == 0)
                unemployedCount++;
        unemploymentRate = (fishers.size() > 0) ? static_cast<double>(unemployedCount) / fishers.size() : 0.0;
        unemploymentHistory.push_back(unemploymentRate);

#if verbose        
        std::cout << "Taux de chômage : " << unemploymentRate * 100 << "%" << std::endl;
#endif
        
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
