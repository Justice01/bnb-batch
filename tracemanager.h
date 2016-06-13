#ifndef TRACEMANAGER_H
#define TRACEMANAGER_H
#include "bnbscheduler.hpp"
#include <QFile>
#include <QStringList>
#include <QDir>
#include <QVector>

#define PROC_PER_FILE 500
#define TIME_PER_FILE 3000

class TraceManager
{
private:
    TraceManager();
public:
    static int parseTrace(const QString fileName, int& procNum, int& maxTime, bool statOnly = false);
    static int countStatistics(double& speedup, double& efficiency, int procNum, int maxTime);
    static int countFirstProblems(const QString fileName, int& firstProblems);
    ~TraceManager();
    struct Conditions
    {
        static const int noaction = -1;
        static const int waiting = 0;
        static const int active = 1;
        static const int sending = 2;
        static const int receiving = 3;
    };
};


#endif // TRACEMANAGER_H
