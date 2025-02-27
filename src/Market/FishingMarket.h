#ifndef FISHINGMARKET_H
#define FISHINGMARKET_H

#include "Market.h"
#include "FishingFirm.h"  // La définition complète de FishingFirm est maintenant disponible.
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <random>
#include <memory>

// La structure FishOffering est déjà définie si FISH_OFFERING_DEFINED est défini.
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

public:
    FishingMarket(double initialClearingPrice = 5.0)
        : Market(initialClearingPrice), matchedVolume(0.0) {}

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

    virtual void clearMarket(std::default_random_engine &generator) override {
        matchedVolume = 0.0;
        double sumTransactionValue = 0.0;
        double totalTransactionVolume = 0.0;

        for (auto &order : orders) {
            for (auto &off : offerings) {
                if (order.desiredSector == off.productSector) {
                    if (order.perceivedValue >= off.offeredPrice) {
                        double transacted = std::min(order.quantity, off.quantity);
                        order.quantity -= transacted;
                        off.quantity -= transacted;
                        matchedVolume += transacted;
                        totalTransactionVolume += transacted;
                        sumTransactionValue += off.offeredPrice * transacted;
                        if (off.firm) {
                            off.firm->addSale(off.offeredPrice, transacted);
                        }
                        if (order.quantity <= 0)
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

    virtual void reset() override {
        Market::reset();
        offerings.clear();
        orders.clear();
        aggregateSupply = 0.0;
        aggregateDemand = 0.0;
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
