#ifndef FISHINGMARKET_H
#define FISHINGMARKET_H

#include "Market.h"
#include "FishingFirm.h"   // full definition of FishingFirm
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <memory>
#include <unordered_map>
#include <iostream>
#include <limits>

// -----------------------------------------------------------------------------
// FishOffering: one supply offer from a firm
// -----------------------------------------------------------------------------
#ifndef FISH_OFFERING_DEFINED
#define FISH_OFFERING_DEFINED
struct FishOffering {
    int id;
    std::string productSector;
    double cost;
    double offeredPrice;
    double quantity;
    std::shared_ptr<FishingFirm> firm;
};
#endif

// -----------------------------------------------------------------------------
// FishOrder: one buy order from a fisherman
// -----------------------------------------------------------------------------
struct FishOrder {
    int id;
    std::string desiredSector;
    double quantity;       // always 1.0 in your simplified model
    double perceivedValue; // max price if not hungry
    bool   hungry;         // if true, bids availableFunds instead
    double availableFunds; // max price if hungry
};

// -----------------------------------------------------------------------------
// FishingMarket: double‐auction market for fish
// -----------------------------------------------------------------------------
class FishingMarket : public Market {
private:
    std::vector<FishOffering>           offerings;
    std::vector<FishOrder>              orders;
    double                              aggregateSupply   = 0.0;
    double                              aggregateDemand   = 0.0;
    double                              matchedVolume     = 0.0;
    std::unordered_map<int,double>      purchases;        // fish bought per fisher
    std::vector<double>                 clearingPrices;   // history

public:
    // Constructor: seed initial clearing price
    FishingMarket(double initialPrice = 5.0)
      : Market(initialPrice)
    {
        clearingPrices.push_back(initialPrice);
    }
    virtual ~FishingMarket() {}

    // --- Accessors ---
    double getClearingFishPrice() const                 { return clearingPrice; }
    double getAggregateSupply()   const                 { return aggregateSupply; }
    double getAggregateDemand()   const                 { return aggregateDemand; }
    double getMatchedVolume()     const                 { return matchedVolume; }
    const std::vector<double>& getClearingPriceHistory()const { return clearingPrices; }
    const std::unordered_map<int,double>& getPurchases()const   { return purchases; }

    // --- Buyer interface ---
    // Submit a buy order and bump aggregateDemand.
    void submitFishOrder(const FishOrder& order) {
        orders.push_back(order);
        aggregateDemand += order.quantity;
    }

    // --- Seller interface ---
    // Collect stock from each firm into 'offerings' and bump aggregateSupply.
    void refreshSupply(const std::vector<std::shared_ptr<FishingFirm>>& firms) {
        offerings.clear();
        aggregateSupply = 0.0;
        for (auto& firm : firms) {
            firm->updateStock();                    // update internal stock
            double stock = firm->getStock();        // retrieve available units

            FishOffering o;
            o.id            = firm->getID();
            o.productSector = "fishing";
            o.cost          = 0.0;
            o.offeredPrice  = firm->getPriceLevel();
            o.quantity      = stock;
            o.firm          = firm;

            offerings.push_back(o);
            aggregateSupply += stock;
        }
    }

    // Return the lowest ask price among current offerings
    double getMinAskPrice() const {
        double minP = std::numeric_limits<double>::infinity();
        for (auto &offer : offerings)
            minP = std::min(minP, offer.offeredPrice);
        return (minP == std::numeric_limits<double>::infinity() ? 0.0 : minP);
    }

    // --- Market clearing: continuous double‐auction ---
    // --- Market clearing: random‐allocation instead of price matching ---
    virtual void clearMarket(std::default_random_engine& generator) override {
        // 1. Reset per‐round stats
        purchases.clear();
        matchedVolume = 0.0;
        double sumValue = 0.0;
        double totalVol = 0.0;

        // 2. Collect all firm IDs and their stock as weights
        std::vector<int> firmIDs;
        std::vector<double> weights;
        for (auto &off : offerings) {
            if (off.quantity > 0.0) {
                firmIDs.push_back(off.id);
                weights.push_back(off.quantity);
            }
        }

        // 3. Distribuer chaque commande aux firmes et tracer par pêcheur
        if (!firmIDs.empty()) {
            std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
            for (const auto &order : orders) {
                if (order.quantity <= 0.0) continue;    // pas de commande
                size_t idx = dist(generator);
                int chosenFirmID = firmIDs[idx];
                // Chercher l'offre correspondante
                for (auto &off : offerings) {
                    if (off.id == chosenFirmID && off.quantity > 0.0) {
                        off.quantity -= 1.0;               // on vend 1 poisson
                        purchases[order.id] += 1.0;        // ← on trace l’achat du pêcheur
                        off.firm->addSale(off.offeredPrice, 1.0);
                        sumValue += off.offeredPrice;
                        totalVol  += 1.0;
                        break;
                    }
                }
            }
        }


        // 4. Compute average clearing price
        if (totalVol > 0.0) {
            clearingPrice = sumValue / totalVol;
        }
        clearingPrices.push_back(clearingPrice);

        // 5. Clean up for next round
        offerings.clear();
        orders.clear();
        aggregateSupply   = 0.0;
        aggregateDemand   = 0.0;
        matchedVolume     = 0.0;
    }


    // --- Reset entire market ---
    virtual void reset() override {
        Market::reset();
        offerings.clear();
        orders.clear();
        aggregateSupply   = 0.0;
        aggregateDemand   = 0.0;
        matchedVolume     = 0.0;
        clearingPrices.clear();
    }

    // Optionally override aggregate demand
    void setAggregateDemand(double d) {
        aggregateDemand = d;
    }

    // Debug print
    virtual void print() const override {
#if verbose == 1
        std::cout << "-----------\nFishing Market State:\n";
        Market::print();
        double totalFish = 0.0;
        for (auto &o : offerings) totalFish += o.quantity;
        std::cout << "Matched Volume: " << matchedVolume << "\n"
                  << "Total Offered : " << totalFish    << "\n"
                  << "Orders Count  : " << orders.size()<< "\n";
#endif
    }
};

#endif // FISHINGMARKET_H
