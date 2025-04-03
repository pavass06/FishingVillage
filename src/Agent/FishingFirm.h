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
#include <numeric> // For accumulate
#include <cstdlib> // For random_shuffle

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
    // Liste des employés (shared pointers to FisherMan)
    std::vector<std::shared_ptr<FisherMan>> employees;
    std::vector<double> revenueHistory;
public:
    // Constructeur: priceLevel fixé à 6.0.
   FishingFirm(int id, double initFunds, int lifetime, int initialEmployees,
                double stock, double salesEfficiency = 2.0)
        : Firm(id, initFunds, lifetime, initialEmployees, stock, 6.0, salesEfficiency, 0.0)
    {
        // La liste employees sera remplie plus tard lors de l'initialisation.
    }


    virtual ~FishingFirm() {}

    // Renvoie la quantité de poissons disponibles à la vente.
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

    // Génération d'une offre d'emploi simple.
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

    // Helper: Compute mean revenue from a vector of firm revenues.
    double computeMeanRevenue(const std::vector<double>& firmRevenues) const {
        double total = std::accumulate(firmRevenues.begin(), firmRevenues.end(), 0.0);
        return total / firmRevenues.size();
    }

    // Helper: Compute the first quartile (25th percentile) of firm revenues.
    double computeFirstQuartile(const std::vector<double>& firmRevenues) const {
        std::vector<double> sortedRevenues = firmRevenues;
        std::sort(sortedRevenues.begin(), sortedRevenues.end());
        size_t n = sortedRevenues.size();
        size_t index = static_cast<size_t>(std::floor(0.25 * n));
        if (index >= n) {
            index = n - 1;
        }
        return sortedRevenues[index];
    }

    // Modified generateJobPostings:
    // 1. Compute mean revenue.
    // 2. Si le revenu de cette firme est supérieur à la moyenne, retourne ceil(log(revenu+1)) offres.
    // 3. Sinon, retourne 0 offres.
    std::vector<JobPosting> generateJobPostings(const std::vector<double>& allFirmRevenues,
                                                const std::string &sector,
                                                int eduReq,
                                                int expReq,
                                                int attract) const {
        std::vector<JobPosting> postings;
        double meanRevenue = computeMeanRevenue(allFirmRevenues);
        double firmRevenue = getRevenue(); // Utilise getRevenue() pour récupérer le revenu mis à jour.
        int numPostings = 0;
        if (firmRevenue > meanRevenue) {
            numPostings = static_cast<int>(std::ceil(std::log(firmRevenue + 1)));
        }
        for (int i = 0; i < numPostings; i++) {
            postings.push_back(generateJobPosting(sector, eduReq, expReq, attract));
        }
        return postings;
    }

    // Nouvelle fonction: generateFiring.
    // Règles:
    // 1. Calculer la moyenne et le premier quartile des revenus.
    // 2. Si le revenu de la firme < premier quartile, licencier aléatoirement ceil(log(revenu+1)) employés.
    // 3. Sinon, ne rien faire.
    void generateFiring(const std::vector<double>& allFirmRevenues) {
        double meanRevenue = computeMeanRevenue(allFirmRevenues);
        double firstQuartileRevenue = computeFirstQuartile(allFirmRevenues);
        double firmRevenue = getRevenue(); // Utilise getRevenue() ici aussi.
        int numToFire = 0;
        if (firmRevenue < firstQuartileRevenue) {
            numToFire = static_cast<int>(std::ceil(std::log(firmRevenue + 1)));
            if (numToFire > static_cast<int>(employees.size())) {
                numToFire = employees.size();
            }
            // Randomiser l'ordre des employés.
            std::shuffle(employees.begin(), employees.end(), std::default_random_engine(std::random_device{}()));
            // Licencier les 'numToFire' premiers employés.
            for (int i = 0; i < numToFire; i++) {
                if (!employees.empty())
                    removeEmployee(employees.front());
            }
        }
    }

    // Ajoute un pêcheur aux employés et met à jour son firmID.
    void addEmployee(std::shared_ptr<FisherMan> emp) {
        // S'assurer que le pêcheur est actuellement au chômage.
        if (emp->getFirmID() == 0) {
            employees.push_back(emp);
            emp->setFirmID(this->getID());
            numberOfEmployees = employees.size();
        }
    }

    // Retire un pêcheur des employés et réinitialise son firmID à 0.
    void removeEmployee(std::shared_ptr<FisherMan> emp) {
        auto it = std::find(employees.begin(), employees.end(), emp);
        if (it != employees.end()) {
            emp->setFirmID(0); // Marquer comme au chômage.
            employees.erase(it);
            numberOfEmployees = employees.size();
        }
    }

    // Licencie un nombre donné d'employés (en supprimant depuis la fin).
    void fireEmployees(int count) {
        for (int i = 0; i < count && !employees.empty(); i++) {
            std::shared_ptr<FisherMan> emp = employees.back();
            emp->setFirmID(0); // Marquer comme au chômage.
            employees.pop_back();
        }
        numberOfEmployees = employees.size();
    }
    
    // Renvoie le nombre d'employés.
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
