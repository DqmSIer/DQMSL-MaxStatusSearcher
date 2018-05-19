#include "Rtree.hpp"

#include "Status.hpp"
#include "StatusInfo.hpp"
#include "BaseStatusList.hpp"

#include "PowerUpper.hpp"
#include "TransUpper.hpp"

#include <iostream>
#include <deque>

using MinCostIndex = Rtree<StatusInfo, Status::STAT_NUM>;
static const char *STAR_CHAR = "★";

BaseStatusList inputBaseStatus(void);
void findMaxStatus(const BaseStatusList &baseStatusList);
void printStatusHistory(const StatusInfo &statusInfo);
std::string makeStarString(const int star, const int transCnt, const bool isCont, const char frontPart, const std::string &contString, const char backPart);

int main(void)
{
	findMaxStatus(inputBaseStatus());
	return 0;
}

BaseStatusList inputBaseStatus(void)
{
	int transCnt = 1;
	BaseStatusList baseStatusList;

	printf("input # of trans: ");
	std::cin >> transCnt;
	puts("");

	baseStatusList.initialize(transCnt);
	baseStatusList.input();
	puts("");

	return baseStatusList;
}

void findMaxStatus(const BaseStatusList &baseStatusList)
{
	const int transCnt = baseStatusList.size();
	std::vector<MinCostIndex> puBaseIndexList(transCnt);
	std::vector<MinCostIndex> puPartnerIndexList(transCnt);
	std::vector<MinCostIndex> trBaseIndexList(transCnt);
	std::deque<StatusInfo> statusTrace;
	StatusInfo maxStatusInfo;

	maxStatusInfo.initialize();
	for (int tc = 0; tc < transCnt; tc++) {
		constexpr auto StatusGetter = [](const StatusInfo &statusInfo) { return &(statusInfo.status[0]); };
		constexpr auto BonusGetter = [](const StatusInfo &statusInfo) { return &(statusInfo.bonus[0]); };
		const Status &baseStatus = baseStatusList[tc];
		MinCostIndex &puBaseIndex = puBaseIndexList[tc];
		MinCostIndex &puPartnerIndex = puPartnerIndexList[tc];
		MinCostIndex &trBaseIndex = trBaseIndexList[tc];

		if (tc == 0) {
			StatusInfo statusInfo;

			statusInfo.star = 0;
			statusInfo.cost = 1;
			statusInfo.status = baseStatus;
			statusInfo.bonus = PowerUpper::getPowerUpValue(baseStatus);
			statusInfo.basePtr = nullptr;
			statusInfo.partnerPtr = nullptr;
			statusInfo.combi = std::string(STAR_CHAR) + std::string("0");

			puBaseIndex.insert(statusInfo, StatusGetter, StatusGetter);
			puPartnerIndex.insert(statusInfo, BonusGetter, BonusGetter);
			statusInfo.bonus.initialize();
			trBaseIndex.insert(statusInfo, BonusGetter, BonusGetter);
		}
		else {
			MinCostIndex &trFormerIndex = trBaseIndexList[tc - 1];
			std::deque<StatusInfo> &trFormerQueue = trFormerIndex.referObjectPool();

			for (const StatusInfo &trFormerInfo: trFormerQueue) {
				StatusInfo statusInfo;

				statusInfo.star = 0;
				statusInfo.cost = trFormerInfo.cost;
				statusInfo.status = baseStatus + trFormerInfo.bonus;
				statusInfo.bonus = PowerUpper::getPowerUpValue(statusInfo.status);

				statusTrace.push_back(trFormerInfo);
				statusInfo.basePtr = &statusTrace.back();
				statusInfo.partnerPtr = nullptr;

				statusInfo.combi = makeStarString(0, tc,
								  !(trFormerInfo.star == 0 && trFormerInfo.cost == 1),
								  '[', trFormerInfo.combi, ']');

				puBaseIndex.insert(statusInfo, StatusGetter, StatusGetter);
				puPartnerIndex.insert(statusInfo, BonusGetter, BonusGetter);
				statusInfo.bonus = TransUpper::getTransUpValue(statusInfo.status, baseStatus);
				trBaseIndex.insert(statusInfo, BonusGetter, BonusGetter);
			}
		}

		for (int star = 0; star < 4; star++) {
			std::deque<StatusInfo> &puBaseQueue = puBaseIndex.referObjectPool();
			std::deque<StatusInfo> &puPartnerQueue = puPartnerIndex.referObjectPool();
			std::deque<StatusInfo> laterUpdateQueue;

			for (const StatusInfo &puBaseInfo: puBaseQueue) {
				/* cannot power up */
				if (puBaseInfo.star < star) {
					continue;
				}

				for (const StatusInfo &puPartnerInfo: puPartnerQueue) {
					StatusInfo nextStatusInfo;

 					if (puBaseInfo.star < puPartnerInfo.star) {
 						continue;
 					}

					nextStatusInfo.star = puBaseInfo.star + 1;
					nextStatusInfo.cost = puBaseInfo.cost + puPartnerInfo.cost;
					nextStatusInfo.status = puBaseInfo.status + puBaseInfo.bonus + puPartnerInfo.bonus; 
					nextStatusInfo.bonus = PowerUpper::getPowerUpValue(nextStatusInfo.status);

					statusTrace.push_back(puBaseInfo);
					nextStatusInfo.basePtr = &statusTrace.back();
					statusTrace.push_back(puPartnerInfo);
					nextStatusInfo.partnerPtr = &statusTrace.back();

					nextStatusInfo.combi = makeStarString(star + 1, tc,
									      !(puBaseInfo.star == puBaseInfo.cost - 1
										&& puPartnerInfo.star == 0
										&& puPartnerInfo.cost == 1),
									      '(', puBaseInfo.combi + std::string("+") + puPartnerInfo.combi, ')');

					if (star < 3) {
						laterUpdateQueue.push_back(nextStatusInfo);
					}
					if (tc < transCnt - 1) {
						nextStatusInfo.bonus = TransUpper::getTransUpValue(nextStatusInfo.status, baseStatus);
						trBaseIndex.update(nextStatusInfo, BonusGetter, BonusGetter,
								   [](const StatusInfo &info, const StatusInfo &newInfo) { 
									   return (newInfo.bonus != info.bonus) ? 0
										   : ((newInfo.cost < info.cost) ? 1 : -1);
								   }
							);
					}
					// if (tc == transCnt - 1 && star == 3) {
					else if (star == 3) {
						if (nextStatusInfo.status > maxStatusInfo.status) {
							maxStatusInfo = nextStatusInfo;
						}
						else if (nextStatusInfo.status >= maxStatusInfo.status && nextStatusInfo.cost <= maxStatusInfo.cost) {
							maxStatusInfo = nextStatusInfo;
						}
					}

				}
			}

			for (const StatusInfo &statusInfo: laterUpdateQueue) {
				puBaseIndex.update(statusInfo, StatusGetter, StatusGetter,
						   [](const StatusInfo &info, const StatusInfo &newInfo) { 
							   return (newInfo.status != info.status) ? 0
								   // : ((newInfo.cost < info.cost && newInfo.star == info.star) ? 1 : -1);
								   : ((newInfo.cost < info.cost) ? 1 : -1);
						   }
					);

				puPartnerIndex.update(statusInfo, BonusGetter, BonusGetter,
						      [](const StatusInfo &info, const StatusInfo &newInfo) { 
							      return (newInfo.bonus != info.bonus) ? 0 : ((newInfo.cost < info.cost) ? 1 : -1);
						      }
					);
			}
			laterUpdateQueue.clear();
		}
	}

	printStatusHistory(maxStatusInfo);

	/* For Debug */
	#if 0
	std::deque<StatusInfo> &tmp = trBaseIndexList[0].referObjectPool();
 	// std::deque<StatusInfo> &tmp = puPartnerIndexList[1].referObjectPool();
 	std::cout << std::endl;
 	for (const StatusInfo &statusInfo: tmp) {
 		std::cout << "----- Trans Base x" << statusInfo.cost << " -----" << std::endl;
 		std::cout << "comb: " << statusInfo.combi << std::endl;
 		statusInfo.printStatusWithBonus();
 	}
	#endif
}

void printStatusHistory(const StatusInfo &statusInfo)
{
	if (statusInfo.basePtr != nullptr) {
		printStatusHistory(*statusInfo.basePtr);

		if (statusInfo.partnerPtr != nullptr) {
			std::cout << "+++++  Power Up With x" << statusInfo.partnerPtr->cost << " +++++" << std::endl;
			std::cout << "comb: " << statusInfo.partnerPtr->combi << std::endl;
			statusInfo.partnerPtr->printStatusWithBonus();
		}
		else {
			std::cout << std::endl;
			std::cout << "******************************" << std::endl;
			std::cout << "********** Trans Up **********" << std::endl;
			std::cout << "******************************" << std::endl;
		}
		std::cout << std::endl;
	}

	std::cout << "----- Current Base x" << statusInfo.cost << " -----" << std::endl;
	std::cout << "comb: " << statusInfo.combi << std::endl;
	statusInfo.printStatusWithBonus();
}

std::string makeStarString(const int star, const int transCnt, const bool isCont, const char frontPart, const std::string &contString, const char backPart)
{
	std::string result;

	result.clear();
	for (int i = 0; i < transCnt + 1; i++) {
		result.append(std::string(STAR_CHAR));
	}
	result.append(std::to_string(star));
	if (isCont == true) {
		result.push_back(frontPart);
		result.append(contString);
		result.push_back(backPart);
	}

	return result;
}
