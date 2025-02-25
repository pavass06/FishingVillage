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
public:
    FishingMarket(double initialClearingPrice = 5.0)
        : Market(initialClearingPrice) {}

    virtual ~FishingMarket() {}

    double getClearingFishPrice() const { return clearingPrice; }

    void submitFishOffering(const FishOffering& offering) {
        offerings.push_back(offering);
        aggregateSupply += offering.quantity;
    }

    void submitFishOrder(const FishOrder& order) {
        orders.push_back(order);
        aggregateDemand += order.quantity;
    }

    virtual void clearMarket(std::default_random_engine &generator) override {
        double matchedVolume = 0.0;
        for (auto &order : orders) {
            for (auto &off : offerings) {
                if (order.desiredSector == off.productSector) {
                    if (order.perceivedValue >= off.offeredPrice) {
                        double transacted = std::min(order.quantity, off.quantity);
                        order.quantity -= transacted;
                        off.quantity -= transacted;
                        matchedVolume += transacted;
                        if (order.quantity <= 0)
                            break;
                    }
                }
            }
        }
        std::cout << "Matched Fish Volume: " << matchedVolume << std::endl;
        if (aggregateDemand > 0) {
            double ratio = aggregateDemand / (aggregateSupply > 0 ? aggregateSupply : 1);
            if (ratio > 1.0) {
                std::normal_distribution<double> adjustDist(1.025, 0.005);
                clearingPrice *= adjustDist(generator);
            } else if (ratio < 1.0) {
                std::normal_distribution<double> adjustDist(0.975, 0.005);
                clearingPrice *= adjustDist(generator);
            }
        }
    }

    virtual void reset() override {
        Market::reset();
        offerings.clear();
        orders.clear();
    }

    virtual void print() const override {
        std::cout << "Fishing Market State:" << std::endl;
        Market::print();
        std::cout << "Number of Fish Offerings: " << offerings.size() << std::endl;
        std::cout << "Number of Fish Orders: " << orders.size() << std::endl;
    }
};

#endif // FISHINGMARKET_H
