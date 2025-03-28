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
    int currentCycle;        // Current simulation day
    int totalCycles;         // Total simulation days
    double annualBirthRate;  // Annual birth rate (e.g., 0.02)
    int maxStarvingDays;     // Maximum consecutive days without food before death

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

    // Nouveau vecteur pour enregistrer le taux de chômage à chaque cycle.
    std::vector<double> unemploymentHistory;

    void updatePopulationAgeDistribution() {
        populationAgeDistribution.clear();
        for (const auto &fisher : fishers) {
            populationAgeDistribution.push_back(fisher->getAge());
        }
    }

    // Paramètres pour la dynamique du marché de l'emploi.
    double postingRate; // par exemple, 0.1 (10% des employés actuels)
    double firingRate;  // par exemple, 0.05 (5% des employés actuels)

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
          double postingRate,    // Posting rate parameter.
          double firingRate)     // Firing rate parameter.
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
            if (!fisher->isEmployed())
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
    
    // Méthode pour récupérer le taux de chômage pour un cycle donné.
    double getUnemployment(int day) const {
        if(day < unemploymentHistory.size())
            return unemploymentHistory[day];
        return 0.0;
    }

    // Méthode pour accéder à l'historique complet du chômage.
    const std::vector<double>& getUnemploymentHistory() const {
        return unemploymentHistory;
    }

    void simulateCycle(std::default_random_engine &generator,
                       std::normal_distribution<double> &firmPriceDist,
                       std::uniform_int_distribution<int> &goodsQuantityDist,
                       std::normal_distribution<double> &consumerPriceDist) {
        std::cout << "----- Début du cycle " << currentCycle + 1 << " -----" << std::endl;

        // 0) Actualiser l'offre.
        if (!firms.empty()) {
            fishingMarket->refreshSupply(firms);
        } else {
            std::cerr << "Erreur : Aucun firm n'est défini dans World." << std::endl;
        }
        if (!firms.empty()) {
            double sumOffered = 0.0;
            for (const auto &firm : firms) {
                sumOffered += firm->getPriceLevel();
            }
            currentOfferMean = sumOffered / firms.size();
        }
        std::cout << "Moyenne des prix offerts : " << currentOfferMean << std::endl;

        // 1) Traitement des actions pour tous les pêcheurs actifs.
        std::cout << "Nombre de pêcheurs avant action : " << fishers.size() << std::endl;
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
        std::cout << "Nombre de pêcheurs après mise à jour : " << fishers.size() << std::endl;

        // 2) Gestion de la population : naissances.
        double normalizedAnnualBirthRate = (annualBirthRate > 1.0) ? annualBirthRate / 100.0 : annualBirthRate;
        double dailyBirthRate = normalizedAnnualBirthRate / 365.0;
        int currentPopulation = getTotalFishers();
        double lambda = dailyBirthRate * currentPopulation;
        std::poisson_distribution<int> poissonDist(lambda);
        int newBirths = poissonDist(generator);
        std::cout << "Nouveaux naissances ce cycle : " << newBirths << std::endl;
        for (int i = 0; i < newBirths; i++) {
            int newID = 1000 + fishers.size();
            auto newFisher = std::make_shared<FisherMan>(
                newID, 10.0, 365 * 60, 0.0, 0.0, 1.0, 1.0, false,
                10.0, 0.0, "fishing", 1, 1, 1
            );
            addFisherMan(newFisher);
        }

        // 3) Processus du marché de l'emploi.
        std::cout << "---- Début du processus du marché de l'emploi ----" << std::endl;
        for (auto &firm : firms) {
            int currentEmployees = firm->getEmployeeCount();
            int fireCount = static_cast<int>(std::ceil(firingRate * currentEmployees));
            firm->fireEmployees(fireCount);
            std::vector<JobPosting> firmPostings = firm->generateJobPostings(postingRate, "fishing", 1, 1, 1);
            std::cout << "Firme " << firm->getID() << " génère " << firmPostings.size() << " offres d'emploi." << std::endl;
            for (const auto &posting : firmPostings) {
                jobMarket->submitJobPosting(posting);
            }
        }

        std::cout << "Nombre total d'offres d'emploi soumises : " 
                  << jobMarket->getMatchedJobs() << std::endl;

        int applicationsCount = 0;
        for (auto &fisher : fishers) {
            if (!fisher->isEmployed()) {
                JobApplication app = fisher->generateJobApplication();
                jobMarket->submitJobApplication(app);
                applicationsCount++;
            }
        }
        std::cout << "Nombre total de candidatures soumises : " << applicationsCount << std::endl;

        jobMarket->clearMarket(generator);
        int matches = jobMarket->getMatchedJobs();
        std::cout << "Nombre de correspondances réalisées : " << matches << std::endl;
        double clearingWage = jobMarket->getClearingWage();
        std::cout << "Wage de clearing : " << clearingWage << std::endl;
        double dailyWage = clearingWage;
        for (auto &fisher : fishers) {
            if (!fisher->isEmployed() && matches > 0) {
                fisher->setEmployed(true);
                fisher->setWage(dailyWage);
                matches--;
            }
        }
        jobMarket->reset();

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

        // 5) Calcul du GDP quotidien.
        double dailyGDP = 0.0;
        for (auto &firm : firms) {
            dailyGDP += firm->getRevenue();
            firm->resetSales();
        }
        GDP = dailyGDP;
        std::cout << "GDP quotidien : " << dailyGDP << std::endl;

        // 6) Calcul du taux de chômage.
        int unemployedCount = 0;
        for (const auto &fisher : fishers)
            if (!fisher->isEmployed())
                unemployedCount++;
        unemploymentRate = (fishers.size() > 0) ? static_cast<double>(unemployedCount) / fishers.size() : 0.0;
        std::cout << "Nombre de pêcheurs au chômage : " << unemployedCount << std::endl;
        std::cout << "Taux de chômage : " << unemploymentRate * 100 << "%" << std::endl;
        
        // Enregistrer le taux de chômage dans l'historique.
        unemploymentHistory.push_back(unemploymentRate);

        // 7) Processus de famine : vérification des commandes.
        std::unordered_map<int, double> purchases = fishingMarket->getPurchases();
        for (auto &fisher : fishers) {
            int fID = fisher->getID();
            if (purchases.find(fID) == purchases.end() || purchases[fID] < 1.0) {
                daysWithoutEat[fID]++;
            } else {
                daysWithoutEat[fID] = 0;
            }
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
        double prevClearingPrice = 0.0;
        double newClearingPrice = 0.0;
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

        // Affichage optionnel de l'état du marché de l'emploi.
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
