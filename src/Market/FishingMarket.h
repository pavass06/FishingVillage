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

    // refreshSupply aggregates available stock from each firm.
    void refreshSupply(const std::vector<std::shared_ptr<FishingFirm>> &firms) {
        // Clear previous offerings and reset the aggregate supply.
        offerings.clear();
        aggregateSupply = 0.0;
        
        // Loop over all firms to update their stock and add to the market offerings.
        for (auto &firm : firms) {
            // Update the firm's stock: deduct sales and add production capacity.
            firm->updateStock();
            double availableSupply = firm->getStock();
            
            // Create a new fish offering based on the firm's current stock.
            FishOffering offer;
            offer.id = firm->getID();
            offer.productSector = "fishing";
            offer.cost = 0; // Adjust cost as needed.
            offer.offeredPrice = firm->getPriceLevel();
            offer.quantity = availableSupply;
            offer.firm = firm;
            
            offerings.push_back(offer);
            aggregateSupply += availableSupply;
        }
    }

    // clearMarket performs order matching with random scanning of offers.
    virtual void clearMarket(std::default_random_engine &generator) override {
        // Clear purchase records and reset matching aggregates.
        purchases.clear();
        matchedVolume = 0.0;
        double sumTransactionValue = 0.0;
        double totalTransactionVolume = 0.0;
        
        // ----- RANDOMIZE THE ORDER -----
        // Shuffle the orders and offerings to avoid fixed ordering biases.
        std::shuffle(orders.begin(), orders.end(), generator);
        std::shuffle(offerings.begin(), offerings.end(), generator);
        
        // Process each order by scanning all offers.
        for (auto &order : orders) {
            // Ensure the order's quantity is set to 1.
            order.quantity = 1.0;
            // Create a list to collect all candidate offers that satisfy the order.
            std::vector<size_t> candidateIndices;
            
            // Loop over all offerings.
            for (size_t i = 0; i < offerings.size(); i++) {
                auto &offer = offerings[i];
                // Check that sector matches and sufficient quantity is available.
                if (order.desiredSector == offer.productSector && offer.quantity >= order.quantity) {
                    // If the fisherman is hungry, check funds; otherwise, check perceived value.
                    if (order.hungry) {
                        if (order.availableFunds >= offer.offeredPrice) {
                            candidateIndices.push_back(i);
                        }
                    }
                    else {
                        if (order.perceivedValue >= offer.offeredPrice) {
                            candidateIndices.push_back(i);
                        }
                    }
                }
            }
            
            // If there is at least one candidate, pick one at random.
            if (!candidateIndices.empty()) {
                std::uniform_int_distribution<size_t> dist(0, candidateIndices.size() - 1);
                size_t chosenIndex = candidateIndices[dist(generator)];
                auto &chosenOffer = offerings[chosenIndex];
                
                // Process the chosen offer: reduce quantities and update aggregate values.
                double transacted = order.quantity; // transacted quantity (1 in our model)
                order.quantity -= transacted;
                chosenOffer.quantity -= transacted;
                matchedVolume += transacted;
                totalTransactionVolume += transacted;
                sumTransactionValue += chosenOffer.offeredPrice * transacted;
                purchases[order.id] += transacted;
                if (chosenOffer.firm) {
                    chosenOffer.firm->addSale(chosenOffer.offeredPrice, transacted);
                }
            }
            // If no candidate is found, the order remains unmatched.
        }
        
        // Calculate the new clearing price if any transaction occurred.
        if (totalTransactionVolume > 0) {
            clearingPrice = sumTransactionValue / totalTransactionVolume;
        }
        
        // Record the clearing price in the history vector.
        clearingPrices.push_back(clearingPrice);
        
        // Reset aggregates for the next cycle.
        aggregateSupply = 0.0;
        aggregateDemand = 0.0;
        matchedVolume = 0.0;
        
        // Clear orders for the next cycle.
        orders.clear();
    }

    // Reset the market state.
    virtual void reset() override {
        Market::reset();
        offerings.clear();
        orders.clear();
        aggregateSupply = 0.0;
        aggregateDemand = 0.0;
        matchedVolume = 0.0;
        clearingPrices.clear(); // Optionally, preserve history.
    }

    // Set aggregate demand externally.
    void setAggregateDemand(double demand) {
        aggregateDemand = demand;
    }

    // Print the market state (for debugging).
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

