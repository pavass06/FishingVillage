#ifndef FISHINGMARKET_H
#define FISHINGMARKET_H

#include "Market.h"
#include "FishingFirm.h"  // Complete definition of FishingFirm is now available.
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <random>
#include <memory>
#include <unordered_map>   // for tracking individual purchases

// Structure for FishOffering (if not defined elsewhere)
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

struct FishOrder {
    int id;
    std::string desiredSector;
    double quantity;
    double perceivedValue;
};

class FishingMarket : public Market {
private:
    std::vector<FishOffering> offerings;
    std::vector<FishOrder> orders;
    double aggregateSupply = 0.0;
    double aggregateDemand = 0.0;
    double matchedVolume;
    
    // NEW: Track individual purchases: fisherID -> totalQuantityBought in this cycle.
    std::unordered_map<int, double> purchases;

public:
    FishingMarket(double initialClearingPrice = 5.0)
        : Market(initialClearingPrice), matchedVolume(0.0)
    {}

    virtual ~FishingMarket() {}

    double getClearingFishPrice() const { return clearingPrice; }
    double getAggregateSupply() const { return aggregateSupply; }
    double getAggregateDemand() const { return aggregateDemand; }

    void submitFishOffering(const FishOffering& offering) {
        offerings.push_back(offering);
        aggregateSupply += offering.quantity;
    }

    void submitFishOrder(const FishOrder& order) {
        orders.push_back(order);
        aggregateDemand += order.quantity;
    }

    // NEW: Return a mapping of fisherID to total purchased quantity.
    const std::unordered_map<int, double>& getPurchases() const {
        return purchases;
    }

    virtual void clearMarket(std::default_random_engine &generator) override {
        // Clear the purchase tracking for this cycle.
        purchases.clear();

        matchedVolume = 0.0;
        double sumTransactionValue = 0.0;
        double totalTransactionVolume = 0.0;

        // Iterate through each order.
        for (auto &order : orders) {
            // For each order, search for a matching offering.
            for (auto &off : offerings) {
                if (order.desiredSector == off.productSector) {
                    // Check that the perceived price is high enough,
                    // the order requests at least 1 unit, and the offering can satisfy the full order.
                    if (order.perceivedValue >= off.offeredPrice && order.quantity >= 1 && off.quantity >= order.quantity) {
                        // Discrete transaction for the entire requested quantity.
                        double transacted = 1;
                        order.quantity -= transacted;
                        off.quantity -= transacted;
                        matchedVolume += transacted;
                        totalTransactionVolume += transacted;
                        sumTransactionValue += off.offeredPrice * transacted;
                        // Record the purchase for this fisherman.
                        purchases[order.id] += transacted;
                        if (off.firm) {
                            off.firm->addSale(off.offeredPrice, transacted);
                        }
                        // Once the order is satisfied, move to the next order.
                        break;
                    }
                }
            }
        }
        
        if (totalTransactionVolume > 0) {
            clearingPrice = sumTransactionValue / totalTransactionVolume;
        }
        aggregateSupply = 0.0;
        aggregateDemand = 0.0;
    }

    // NEW: Reset function to clear orders (and offerings) at the end of the cycle.
    virtual void reset() override {
        // Call base class reset (if necessary).
        Market::reset();
        // Clear the vectors so orders from previous cycles don't accumulate.
        offerings.clear();
        orders.clear();
        aggregateSupply = 0.0;
        aggregateDemand = 0.0;
        // Optionally, reset matchedVolume.
        matchedVolume = 0.0;
    }

    virtual void print() const override {
        std::cout << "-----------" << std::endl;
        std::cout << "Fishing Market State:" << std::endl;
        Market::print();
        double totalFishProvided = 0.0;
        for (const auto &offering : offerings) {
            totalFishProvided += offering.quantity;
        }
        std::cout << "Matched Fish Volume: " << matchedVolume << std::endl;
        std::cout << "Total Fish Provided: " << totalFishProvided << std::endl;
        std::cout << "Number of Fish Orders: " << orders.size() << std::endl;
    }
};

#endif // FISHINGMARKET_H
