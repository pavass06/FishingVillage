#ifndef FISHINGMARKET_H
#define FISHINGMARKET_H

#include "Market.h"
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <random>

struct FishOffering {
    int id;
    std::string productSector;
    double cost;
    double offeredPrice;
    double quantity;
};

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
    // New members to track aggregate supply and demand:
    double aggregateSupply = 0.0;
    double aggregateDemand = 0.0;
public:
    FishingMarket(double initialClearingPrice = 5.0)
        : Market(initialClearingPrice) {}

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

    // clearMarket calculates the new clearing price as the weighted mean of all deal prices.
    virtual void clearMarket(std::default_random_engine &generator) override {
        double matchedVolume = 0.0;
        double sumTransactionValue = 0.0; // Sum of (deal price * transaction volume)
        double totalTransactionVolume = 0.0; // Total volume transacted

        // Match orders with offerings.
        for (auto &order : orders) {
            for (auto &off : offerings) {
                if (order.desiredSector == off.productSector) {
                    // Transaction occurs if the consumer's perceived price meets or exceeds the firm's offered price.
                    if (order.perceivedValue >= off.offeredPrice) {
                        double transacted = std::min(order.quantity, off.quantity);
                        order.quantity -= transacted;
                        off.quantity -= transacted;
                        matchedVolume += transacted;
                        totalTransactionVolume += transacted;
                        // Assume the transaction price is the firm's offered price.
                        sumTransactionValue += off.offeredPrice * transacted;
                        if (order.quantity <= 0)
                            break;
                    }
                }
            }
        }
        std::cout << "Matched Fish Volume: " << matchedVolume << std::endl;
        
        // Set the clearing price to the weighted average of all deal prices.
        if (totalTransactionVolume > 0) {
            clearingPrice = sumTransactionValue / totalTransactionVolume;
        }
        // Reset aggregate values for the next cycle.
        aggregateSupply = 0.0;
        aggregateDemand = 0.0;
    }

    virtual void reset() override {
        Market::reset();
        offerings.clear();
        orders.clear();
        aggregateSupply = 0.0;
        aggregateDemand = 0.0;
    }

    virtual void print() const override {
        std::cout << "Fishing Market State:" << std::endl;
        Market::print();
        std::cout << "Number of Fish Offerings: " << offerings.size() << std::endl;
        std::cout << "Number of Fish Orders: " << orders.size() << std::endl;
    }
};

#endif // FISHINGMARKET_H
