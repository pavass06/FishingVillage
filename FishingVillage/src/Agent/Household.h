#ifndef HOUSEHOLD_H
#define HOUSEHOLD_H

#include "Agent.h"
#include <iostream>

class Household : public Agent {
protected:
    double income;      // Earnings (wages or dividends)
    double savings;     // Accumulated wealth
    double jobDemand;   // Indicator/quantity for job seeking
    double goodsDemand; // Quantity of goods desired

public:
    Household(int id, double initFunds, int lifetime, double income, double savings, double jobDemand, double goodsDemand)
        : Agent(id, initFunds, lifetime), income(income), savings(savings), jobDemand(jobDemand), goodsDemand(goodsDemand) {}

    virtual ~Household() {}

    // Inherit act() and update() from Agent; these can be extended in derived classes.
    virtual void print() const override {
        Agent::print();
        std::cout << "Income: " << income 
                  << " | Savings: " << savings 
                  << " | Job Demand: " << jobDemand 
                  << " | Goods Demand: " << goodsDemand << std::endl;
    }

    // Getters and Setters
    double getIncome() const { return income; }
    void setIncome(double inc) { income = inc; }

    double getSavings() const { return savings; }
    void setSavings(double s) { savings = s; }

    double getJobDemand() const { return jobDemand; }
    void setJobDemand(double jd) { jobDemand = jd; }

    double getGoodsDemand() const { return goodsDemand; }
    void setGoodsDemand(double gd) { goodsDemand = gd; }
};

#endif // HOUSEHOLD_H
