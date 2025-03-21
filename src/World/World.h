#ifndef WORLD_H
#define WORLD_H

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstdlib>
#include <random>
#include <unordered_map>
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

    // Population vector.
    std::vector<std::shared_ptr<FisherMan>> fishers;
    // Pointer to vector of FishingFirm objects.
    const std::vector<std::shared_ptr<FishingFirm>>* firms;

    std::shared_ptr<JobMarket> jobMarket;
    std::shared_ptr<FishingMarket> fishingMarket;

    // Macro variables.
    double GDP;
    double unemploymentRate;
    double inflation;

    // Inflation calculation variables.
    double currentOfferMean;
    double currentPerceivedMean;
    // Inflation adjustment parameters:
    double meanAugmentationInflat;
    double varianceAugmentationInflat;
    double meanDiminutionInflat;
    double varianceDiminutionInflat;
    // (Optional) A vector to store daily inflation values.
    std::vector<double> inflations;

    // Mapping: fisherID -> consecutive days without eating.
    std::unordered_map<int, int> daysWithoutEat;

public:
    // Constructor now accepts additional parameters for inflation.
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
          double varDimIn)
        : currentCycle(0),
          totalCycles(cycles),
          annualBirthRate(annualBirthRate_),
          maxStarvingDays(maxStarvingDays_),
          jobMarket(jm),
          fishingMarket(fm),
          GDP(0.0),
          unemploymentRate(0.0),
          inflation(0.0),
          firms(nullptr),
          currentOfferMean(offeredPriceMean),
          currentPerceivedMean(perceivedPriceMean),
          meanAugmentationInflat(meanAugIn),
          varianceAugmentationInflat(varAugIn),
          meanDiminutionInflat(meanDimIn),
          varianceDiminutionInflat(varDimIn)
    {}

    // Setter for firms vector.
    void setFirms(const std::vector<std::shared_ptr<FishingFirm>> &firmVec) {
         firms = &firmVec;
    }

    // Add a fisherman to the world.
    void addFisherMan(std::shared_ptr<FisherMan> f) {
        fishers.push_back(f);
        daysWithoutEat[f->getID()] = 0;
    }

    int getTotalFishers() const {
        return static_cast<int>(fishers.size());
    }

    double getGDP() const { return GDP; }

    double getUnemploymentRate () const { return unemploymentRate; }
    
    double getInflation(int day) const {return inflations[day];}

    int getUnemployedFishers() const {
        int count = 0;
        for (const auto &fisher : fishers)
            if (!fisher->isEmployed())
                count++;
        return count;
    }

    // The core simulation cycle.
    void simulateCycle(std::default_random_engine &generator,
                   std::normal_distribution<double> &firmPriceDist,
                   std::uniform_int_distribution<int> &goodsQuantityDist,
                   std::normal_distribution<double> &consumerPriceDist) {
    // 0) Refresh supply: mettre à jour le stock de chaque firme et générer de nouvelles offres.
    if (firms != nullptr) {
        fishingMarket->refreshSupply(*firms);
    } else {
        std::cerr << "Error: Firms vector not set in World." << std::endl;
    }

    // Mise à jour de la moyenne des prix offerts (currentOfferMean) à partir des firmes.
    if (firms != nullptr && !firms->empty()) {
        double sumOffered = 0.0;
        for (const auto &firm : *firms) {
            // On suppose que firm->getPriceLevel() retourne le prix offert par la firme.
            sumOffered += firm->getPriceLevel();
        }
        currentOfferMean = sumOffered / firms->size();
    }

    // 1) Traitement des actions pour tous les pêcheurs actifs.
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

    // 2) Gestion de la population: naissances via un processus de Poisson.
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
    if (firms != nullptr) {
        for (auto &firm : *firms) {
            JobPosting posting = firm->generateJobPosting("fishing", 1, 1, 1);
            jobMarket->submitJobPosting(posting);
        }
    }
    for (auto &fisher : fishers) {
        if (!fisher->isEmployed()) {
            JobApplication app = fisher->generateJobApplication();
            jobMarket->submitJobApplication(app);
        }
    }
    jobMarket->clearMarket(generator);
    double clearingWage = jobMarket->getClearingWage();
    double dailyWage = 1.0 * clearingWage;
    int matches = jobMarket->getMatchedJobs();
    for (auto &fisher : fishers) {
        if (!fisher->isEmployed() && matches > 0) {
            fisher->setEmployed(true);
            fisher->setWage(dailyWage);
            matches--;
        }
    }
    jobMarket->reset();

    // 4) Ordres du marché de la pêche: chaque pêcheur passe commande pour 1 poisson.
    double sumPerceived = 0.0;
    int orderCount = 0;
    for (auto &fisher : fishers) {
        FishOrder order;
        order.id = fisher->getID();
        order.desiredSector = "fishing";
        order.quantity = 1.0;
        // Génère le prix perçu à partir de la distribution normale consumerPriceDist.
        order.perceivedValue = consumerPriceDist(generator);
        sumPerceived += order.perceivedValue;
        orderCount++;
        order.availableFunds = fisher->getFunds();
        order.hungry = (daysWithoutEat[fisher->getID()] > 0);
        fishingMarket->submitFishOrder(order);
    }
    // Calcul de la moyenne des prix perçus.
    if (orderCount > 0)
        currentPerceivedMean = sumPerceived / orderCount;
    else
        currentPerceivedMean = 0.0;

    // Force la demande agrégée à être égale au nombre total de pêcheurs.
    fishingMarket->setAggregateDemand(static_cast<double>(getTotalFishers()));
    fishingMarket->clearMarket(generator);
    // Attention : la méthode reset() du FishingMarket peut vider l'historique des clearing prices.
    // Si vous souhaitez conserver cet historique pour le calcul de l'inflation, il faudra éviter de le réinitialiser ici.
    // Pour l'exemple, nous commentons la ligne suivante :
    // fishingMarket->reset();

    // 5) Calcul du PIB quotidien: somme des revenus de chaque firme.
    double dailyGDP = 0.0;
    if (firms != nullptr) {
        for (auto &firm : *firms) {
            dailyGDP += firm->getRevenue();
            firm->resetSales();
        }
    }
    GDP = dailyGDP;

    // 6) Calcul du taux de chômage.
    int unemployedCount = 0;
    for (const auto &fisher : fishers)
        if (!fisher->isEmployed())
            unemployedCount++;
    unemploymentRate = (fishers.size() > 0) ? static_cast<double>(unemployedCount) / fishers.size() : 0.0;

    // 7) Processus de la famine: vérifie que chaque pêcheur a bien acheté du poisson.
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
        if (daysWithoutEat[fID] >= maxStarvingDays)
            fisher->setActive(false);
    }
    fishers.erase(std::remove_if(fishers.begin(), fishers.end(),
        [](const std::shared_ptr<FisherMan>& f) { return !f->isActive(); }),
        fishers.end());

    // 8) Calcul de l'inflation.
    // On se base sur l'évolution du clearing price dans le FishingMarket.
    double prevClearingPrice = 0.0;
    double newClearingPrice = 0.0;
    const std::vector<double>& priceHistory = fishingMarket->getClearingPriceHistory();
    if (priceHistory.size() >= 2) {
        prevClearingPrice = priceHistory[priceHistory.size() - 2];
        newClearingPrice = priceHistory.back();
        double inflRate = (newClearingPrice - prevClearingPrice) / prevClearingPrice;
        inflation = inflRate;
        inflations.push_back(inflation);
    } else {
        inflation = 0.0;
        inflations.push_back(0.0);
    }
   std::cout << "Clearing price (n+1): " << newClearingPrice 
          << " | Clearing price (n): " << prevClearingPrice 
          << " | inflation: " << inflation*100
          << std::endl;

    currentCycle++;
}



    void runSimulation(std::default_random_engine &generator,
                       std::normal_distribution<double> &firmPriceDist,
                       std::uniform_int_distribution<int> &goodsQuantityDist,
                       std::normal_distribution<double> &consumerPriceDist) {
        for (int day = 0; day < totalCycles; day++) {
            simulateCycle(generator, firmPriceDist, goodsQuantityDist, consumerPriceDist);
        }
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
