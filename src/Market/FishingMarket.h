#ifndef FISHINGMARKET_H
#define FISHINGMARKET_H

#include "Market.h"
#include "FishingFirm.h"  // Complete definition of FishingFirm is available.
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
    double quantity;       // In our simplified model, each order has quantity = 1.
    double perceivedValue;
    bool hungry;           // True if the fisherman did not eat yesterday.
    double availableFunds; // Funds available when the order is created.
};

class FishingMarket : public Market {
private:
    std::vector<FishOffering> offerings;
    std::vector<FishOrder> orders;
    double aggregateSupply = 0.0;
    double aggregateDemand = 0.0;
    double matchedVolume = 0.0;
    
    // Map from fisherID to the total purchased quantity in this cycle.
    std::unordered_map<int, double> purchases;
    
    // Vector storing the history of clearing prices.
    std::vector<double> clearingPrices;

public:
    FishingMarket(double initialClearingPrice = 5.0)
        : Market(initialClearingPrice)
    {
        // Initialize the clearing price history with the initial price.
        clearingPrices.push_back(initialClearingPrice);
    }

    virtual ~FishingMarket() {}

    
    double getClearingFishPrice() const { return clearingPrice; }
    double getAggregateSupply() const { return aggregateSupply; }
    double getAggregateDemand() const { return aggregateDemand; }
    double getMatchedVolume() const { return matchedVolume; }
    const std::vector<double>& getClearingPriceHistory() const { return clearingPrices; }
    const std::unordered_map<int, double>& getPurchases() const {
        return purchases;
    }

    // Submit a fish order. This adds the order and updates the aggregate demand.
    void submitFishOrder(const FishOrder& order) {
        orders.push_back(order);
        aggregateDemand += order.quantity;
    }

    
    // Each firm updates its stock (deducting sold fish and adding new production) via updateStock().
    // Then, the firm's current stock (obtained with getStock()) is used as the available supply for this cycle.
    void refreshSupply(const std::vector<std::shared_ptr<FishingFirm>> &firms) {
        // Clear previous offerings and reset the aggregate supply.
        offerings.clear();
        aggregateSupply = 0.0;
        
        // Loop over all firms to update their stock and add to the market offerings.
        for (auto &firm : firms) {
            // Update the firm's stock: this deducts the sold quantity and adds the production capacity.
            firm->updateStock();
            
            // Retrieve the firm's updated stock.
            double availableSupply = firm->getStock();
            
            // Create a new fish offering based on the firm's current stock.
            FishOffering offer;
            offer.id = firm->getID();
            offer.productSector = "fishing";
            offer.cost = 0; // Adjust cost as needed.
            offer.offeredPrice = firm->getPriceLevel();
            offer.quantity = availableSupply;
            offer.firm = firm;
            
            // Add the offering to the list.
            offerings.push_back(offer);
            
            // Accumulate the total supply available in the market.
            aggregateSupply += availableSupply;
        }
    }


    // clearMarket performs order matching.
    virtual void clearMarket(std::default_random_engine &generator) override {
        // Clear purchase records from the previous cycle.
        purchases.clear();
        matchedVolume = 0.0;
        double sumTransactionValue = 0.0;
        double totalTransactionVolume = 0.0;

        // Process each order.
        for (auto &order : orders) {
            // Force each order's quantity to 1.
            order.quantity = 1.0;
            for (auto &off : offerings) {
                if (order.desiredSector == off.productSector) {
                    // If the fisherman is hungry, he buys automatically if funds allow.
                    if (order.hungry) {
                        if (order.availableFunds >= off.offeredPrice && off.quantity >= order.quantity) {
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
                    // Otherwise, the fisherman buys only if the perceived price is at least the offered price.
                    else {
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
        
        // Calculate the clearing price if any transaction occurred.
        if (totalTransactionVolume > 0) {
            clearingPrice = sumTransactionValue / totalTransactionVolume;
        }
        
        // Record the new clearing price in the history vector.
        clearingPrices.push_back(clearingPrice);
        
        // Reset aggregates for the next cycle.
        aggregateSupply = 0.0;
        aggregateDemand = 0.0;
        matchedVolume = 0.0;
        
        // Clear orders for the next cycle.
        orders.clear();
    }

    // Reset the market: clear offerings, orders, aggregates, and optionally the clearing price history.
    virtual void reset() override {
        Market::reset();
        offerings.clear();
        orders.clear();
        aggregateSupply = 0.0;
        aggregateDemand = 0.0;
        matchedVolume = 0.0;
        clearingPrices.clear(); // Optionally, preserve the history if desired.
    }

    // Set aggregate demand (used externally to force a value).
    void setAggregateDemand(double demand) {
        aggregateDemand = demand;
    }

    // Print the market state (debug mode).
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
