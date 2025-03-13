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
#include <unordered_map>

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
    double quantity;       // In our simplified model, each order should have quantity = 1.
    double perceivedValue;
    bool hungry;           // True if the fisherman has not eaten for at least one day.
    double availableFunds; // Funds available when the order is created.
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

    // Return mapping of fisherID to total purchased quantity.
    const std::unordered_map<int, double>& getPurchases() const {
        return purchases;
    }

    virtual void clearMarket(std::default_random_engine &generator) override {
        // Clear purchase tracking from the previous cycle.
        purchases.clear();

        matchedVolume = 0.0;
        double sumTransactionValue = 0.0;
        double totalTransactionVolume = 0.0;

        // --- DEBUG: Print out the aggregate demand and supply before matching ---
        std::cout << "DEBUG: Before matching, Aggregate Supply = " << aggregateSupply 
                  << ", Aggregate Demand = " << aggregateDemand << std::endl;
        // In our simplified model, each fisher places an order for exactly 1 fish,
        // so aggregateDemand should equal the number of fishers.
        // AggregateSupply is the sum of all offered quantities from firms.

        // Process each order.
        for (auto &order : orders) {
            // (Optionally force each order to have quantity 1, if not already)
            order.quantity = 1.0;
            for (auto &off : offerings) {
                if (order.desiredSector == off.productSector) {
                    if (order.hungry) {
                        if (order.availableFunds >= off.offeredPrice && off.quantity >= order.quantity) {
                            double transacted = order.quantity;  // Fulfill entire order.
                            order.quantity -= transacted;
                            off.quantity -= transacted;
                            matchedVolume += transacted;
                            totalTransactionVolume += transacted;
                            sumTransactionValue += off.offeredPrice * transacted;
                            purchases[order.id] += transacted;
                            if (off.firm) {
                                off.firm->addSale(off.offeredPrice, transacted);
                            }
                            break;
                        }
                    } else {
                        if (order.perceivedValue >= off.offeredPrice && off.quantity >= order.quantity) {
                            double transacted = order.quantity;
                            order.quantity -= transacted;
                            off.quantity -= transacted;
                            matchedVolume += transacted;
                            totalTransactionVolume += transacted;
                            sumTransactionValue += off.offeredPrice * transacted;
                            purchases[order.id] += transacted;
                            if (off.firm) {
                                off.firm->addSale(off.offeredPrice, transacted);
                            }
                            break;
                        }
                    }
                }
            }
        }
        
        if (totalTransactionVolume > 0) {
            clearingPrice = sumTransactionValue / totalTransactionVolume;
        }
        // Debug prints after matching.
        std::cout << "DEBUG: Matched Volume = " << matchedVolume << std::endl;
        std::cout << "DEBUG: New Clearing Price = " << clearingPrice << std::endl;
        
        // Reset aggregates for next cycle.
        aggregateSupply = 0.0;
        aggregateDemand = 0.0;
    }

    virtual void reset() override {
        Market::reset();
        offerings.clear();
        orders.clear();
        aggregateSupply = 0.0;
        aggregateDemand = 0.0;
        matchedVolume = 0.0;
    }

    virtual void print() const override {
#if verbose==1
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
#endif
    }
};

#endif // FISHINGMARKET_H
