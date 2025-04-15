#ifndef FISHINGFIRM_H
#define FISHINGFIRM_H

#include "Firm.h"
#include "FisherMan.h"
#include "JobMarket.h"  // For JobPosting struct

#include <algorithm>
#include <iostream>
#include <cmath>
#include <memory>
#include <string>
#include <vector>
#include <cstdlib> // For random_shuffle
#include <random>  // Pour std::default_random_engine, std::random_device

#ifndef FISH_OFFERING_DEFINED
#define FISH_OFFERING_DEFINED

class FisherMan;

struct FishOffering {
    int id;
    std::string productSector;
    double cost;
    double offeredPrice;
    double quantity;
    std::shared_ptr<class FishingFirm> firm;
};
#endif

class FishingFirm : public Firm {
private:
    // List of employees (shared pointers to FisherMan)
    std::vector<std::shared_ptr<FisherMan>> employees;
    // Revenue history: each firm records its current revenue per cycle.
    std::vector<double> revenueHistory;
public:
    // Constructor: priceLevel fixed to 6.0
    FishingFirm(int id, double initFunds, int lifetime, int initialEmployees,
                double stock, double salesEfficiency = 2.0)
        : Firm(id, initFunds, lifetime, initialEmployees, stock, 6.0, salesEfficiency, 0.0)
    {}

    virtual ~FishingFirm() {}

    // Returns the amount of fish available for sale.
    virtual double getGoodsSupply() const {
        return std::min(stock, salesEfficiency * static_cast<double>(employees.size()));
    }

    virtual FishOffering generateGoodsOffering(double cost) const {
        FishOffering offer;
        offer.id = getID();
        offer.productSector = "fishing";
        offer.cost = cost;
        offer.offeredPrice = getPriceLevel();
        offer.quantity = getGoodsSupply();
        return offer;
    }

    // Generates a single job posting.
    virtual JobPosting generateJobPosting(const std::string &sector, int eduReq, int expReq, int attract) const override {
        JobPosting posting;
        posting.firmID = getID();
        posting.jobSector = sector;
        posting.educationRequirement = eduReq;
        posting.experienceRequirement = expReq;
        posting.attractiveness = attract;
        posting.vacancies = 1;
        posting.recruiting = (posting.vacancies > 0);
        return posting;
    }

    // New generateJobPostings method based on third quartile revenue.
    // If the firm's revenue is greater than the global third quartile, it returns a number 
    // of job postings equal to ceil(log(revenue + 1)); otherwise, 5% chance to still post.
    std::vector<JobPosting> generateJobPostings(double thirdQuartile,
                                            const std::string &sector,
                                            int eduReq,
                                            int expReq,
                                            int attract) const {
        std::vector<JobPosting> postings;
        double firmRev = getCurrentFirmRevenue();

        bool forcePost = ((double) rand() / RAND_MAX < 0.05);
        int numPostings = 0;
        if (firmRev > thirdQuartile) {
            numPostings = static_cast<int>(std::ceil(std::log(firmRev + 1)));
        }

        for (int i = 0; i < numPostings; i++) {
            postings.push_back(generateJobPosting(sector, eduReq, expReq, attract));
        }

        return postings;
    }

    // Record the current revenue in the history.
    void recordRevenue() {
        revenueHistory.push_back(getRevenue());
    }

    const std::vector<double>& getRevenueHistory() const {
        return revenueHistory;
    }

    double getCurrentFirmRevenue() const {
        return (!revenueHistory.empty() ? revenueHistory.back() : getRevenue());
    }

    // Modified generateFiring method based on first quartile revenue.
    void generateFiring(double firstQuartile) {
        double firmRev = getCurrentFirmRevenue();

        bool forceFire = ((double) rand() / RAND_MAX < 0.05);
        if (firmRev < firstQuartile) {
            int numToFire = static_cast<int>(std::ceil(std::log(firmRev + 1)));
            if (numToFire > static_cast<int>(employees.size())) {
                numToFire = employees.size();
            }
            std::shuffle(employees.begin(), employees.end(), std::default_random_engine(std::random_device{}()));
            for (int i = 0; i < numToFire; i++) {
                if (!employees.empty())
                    removeEmployee(employees.front());
            }
        }
    }

    void addEmployee(std::shared_ptr<FisherMan> emp) {
        if (emp->getFirmID() == 0) {
            employees.push_back(emp);
            emp->setFirmID(this->getID());
            numberOfEmployees = employees.size();
        }
    }

    void removeEmployee(std::shared_ptr<FisherMan> emp) {
        auto it = std::find(employees.begin(), employees.end(), emp);
        if (it != employees.end()) {
            emp->setFirmID(0);
            employees.erase(it);
            numberOfEmployees = employees.size();
        }
    }

    void fireEmployees(int count) {
        for (int i = 0; i < count && !employees.empty(); i++) {
            std::shared_ptr<FisherMan> emp = employees.back();
            emp->setFirmID(0);
            employees.pop_back();
        }
        numberOfEmployees = employees.size();
    }

    int getEmployeeCount() const { 
        return employees.size();
    }

    virtual void print() const override {
        Firm::print();
        std::cout << "Goods Supply (Fish Available): " << getGoodsSupply() << std::endl;
        std::cout << "Nombre d'employés: " << getEmployeeCount() << std::endl;
    }
};

#endif // FISHINGFIRM_H
