#ifndef FIRM_H
#define FIRM_H

#include "Agent.h"
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include "JobMarket.h"  // For JobPosting struct

class Firm : public Agent {
protected:
    int numberOfEmployees;    // Number of workers employed by the firm
    double stock;             // Current product inventory
    double priceLevel;        // Offered price per fish (e.g., ~6 pounds)
    double salesEfficiency;   // α: Sales efficiency factor (units each employee can sell)
    double jobPostMultiplier; // δ: Multiplier for number of job posts
    double wageExpense;       // Computed as numberOfEmployees * clearing wage

public:
    // Constructor with 8 parameters.
    Firm(int id, double initFunds, int lifetime, int numberOfEmployees,
         double stock, double priceLevel,
         double salesEfficiency = 2.0, double jobPostMultiplier = 1.05)
         : Agent(id, initFunds, lifetime),
           numberOfEmployees(numberOfEmployees),
           stock(stock),
           priceLevel(priceLevel),
           salesEfficiency(salesEfficiency),
           jobPostMultiplier(jobPostMultiplier),
           wageExpense(0.0)
    {}

    virtual ~Firm() {}

    virtual double calculateRevenue() const {
         double quantitySold = std::min(stock, salesEfficiency * numberOfEmployees);

     //     std::cout << " numberOfEmployees " <<  numberOfEmployees << std::endl;
     //     std::cout << " stock " <<  stock << std::endl;
     //     std::cout << " Sold " << quantitySold << std::endl;
     //     std::cout << " price level " << priceLevel<< std::endl;
         
         return quantitySold * priceLevel;
    }

    void setWageExpense(double clearingWage) {
         wageExpense = numberOfEmployees * clearingWage;
    }

    virtual double calculateProfit() const {
         return calculateRevenue() - wageExpense;
    }

    virtual double investmentExpenditure() const {
         double profit = calculateProfit();
         if(profit <= 0)
             return 0;
         double s = static_cast<double>(rand()) / RAND_MAX;
         return profit * (1.0 - s);
    }

    virtual void act() override {
         double invest = investmentExpenditure();
         stock += invest; 
         stock = std::max(stock,0.0); // Ensure stock never goes negative.
         funds += calculateRevenue() - wageExpense;
    }

    virtual void update() override {
         Agent::update();
    }

    virtual void print() const override {
         Agent::print();
         std::cout << "Employees: " << numberOfEmployees 
                   << " | Stock: " << stock 
                   << " | Price Level: " << priceLevel 
                   << " | Sales Efficiency (α): " << salesEfficiency 
                   << " | Job Post Multiplier (δ): " << jobPostMultiplier 
                   << " | Wage Expense: " << wageExpense 
                   << " | Revenue: " << calculateRevenue() 
                   << " | Profit: " << calculateProfit() << std::endl;
    }

    // Getters and setters.
    int getNumberOfEmployees() const { return numberOfEmployees; }
    void setNumberOfEmployees(int ne) { numberOfEmployees = ne; }

    double getStock() const { return stock; }
    void setStock(double s) { stock = s; }

    double getPriceLevel() const { return priceLevel; }
    void setPriceLevel(double p) { priceLevel = p; }

    double getSalesEfficiency() const { return salesEfficiency; }
    void setSalesEfficiency(double se) { salesEfficiency = se; }

    double getJobPostMultiplier() const { return jobPostMultiplier; }
    void setJobPostMultiplier(double jpm) { jobPostMultiplier = jpm; }

    virtual double getRevenue() const { return calculateRevenue(); }
    
    // Declare a pure virtual method so that derived classes must implement it.
    virtual JobPosting generateJobPosting(const std::string &sector, int eduReq, int expReq, int attract) const = 0;
};

#endif // FIRM_H
