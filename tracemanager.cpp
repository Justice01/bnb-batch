#include "tracemanager.h"
TraceManager::TraceManager()
{

}

TraceManager::~TraceManager()
{

}

int TraceManager::parseTrace(const QString fileName, int& procNum, int& maxTime, bool statOnly)
{

    bool procNumSet=true;
    if (procNum==0)procNumSet=false;
    QFile trace(fileName);
    //getting maxTime and procNum
    if(trace.open(QIODevice::ReadOnly|QIODevice::Text))
    {

        QString line;
        QStringList traceLine;
        while(!trace.atEnd())
        {
            line = trace.readLine();
            traceLine = line.split(' ');
            if(traceLine.length()>2 && !procNumSet)
            {
                if(traceLine.at(1).toInt()>=procNum && traceLine.at(2).toInt()==6) procNum=traceLine.at(1).toInt()+1;
                else if(traceLine.at(0).toInt()>0) procNumSet = true;
            }
            if(trace.atEnd())maxTime = traceLine.at(0).toInt();
        }
        trace.close();
    }

    //parsing trace

    QDir dir;
    if(!dir.exists("traces")){dir.mkdir("traces");}
    if(!dir.exists("traces/subtraces")){dir.mkdir("traces/subtraces");}
    dir.setPath("traces/subtraces");
    QStringList lst = dir.entryList(QDir::Files);
    if(lst.count()>0)
    {
        for (int i=0; i<lst.length();i++)
        {
            QFile::remove(dir.absolutePath().append("/").append(lst.at(i)));
        }
    }

    //creating subtraces
    int currentMinProc = 0;
    int currentMinTime = 0;
    int currentMaxProc = 0;
    int currentMaxTime = 0;
    QVector <int> solves(PROC_PER_FILE);
    QVector <int> sends(PROC_PER_FILE);
    QVector <int> recvs(PROC_PER_FILE);
    QVector <int> conditions(PROC_PER_FILE,-1);
    QVector <int> sequentalTime(PROC_PER_FILE);
    QVector< QVector<int> > procs(PROC_PER_FILE, QVector<int>(TIME_PER_FILE+1));

    while(currentMaxProc<procNum-1)
    {
        currentMaxProc= currentMinProc + PROC_PER_FILE - 1;
        if(currentMaxProc>procNum-1)
        {
            currentMaxProc=procNum-1;
            solves.resize(currentMaxProc-currentMinProc+1);
            solves.squeeze();
            sends.resize(currentMaxProc-currentMinProc+1);
            sends.squeeze();
            recvs.resize(currentMaxProc-currentMinProc+1);
            recvs.squeeze();
            conditions.resize(currentMaxProc-currentMinProc+1);
            conditions.squeeze();
            sequentalTime.resize(currentMaxProc-currentMinProc+1);
            sequentalTime.squeeze();
            procs.resize(currentMaxProc-currentMinProc+1);
            procs.squeeze();
        }

        for(int i = 0; i<procs.length();i++)
        {
            procs[i].fill(0);
        }

        QString line;
        QStringList traceLine;

        bool isNextFile = false;
        if(trace.open(QIODevice::ReadOnly|QIODevice::Text))
        {
            currentMinTime=0;
            currentMaxTime=0;
            for(int i = 0; i<procs.length(); i++)
            {
                procs[i].resize(TIME_PER_FILE+1);
                procs.squeeze();
            }
            while(currentMaxTime<maxTime)
            {

                for(int i=0; i<procs.length();i++ )
                    for(int j=0;j<procs[i].length();j++)
                    {
                        if(j==0)sequentalTime[i]=0;
                        procs[i][j]=0;
                    }
                currentMinTime=currentMaxTime;
                currentMaxTime+=TIME_PER_FILE;

                if(currentMaxTime>maxTime)
                {
                    currentMaxTime=maxTime;
                    for(int i = 0; i<procs.length(); i++)
                    {
                        procs[i].resize(currentMaxTime - currentMinTime + 1);
                        procs.squeeze();
                    }
                }
                QFile partTrace(QString("traces/subtraces/").append(QString::number(currentMinProc)).append("_").append(QString::number(currentMaxProc)).append("_").append(QString::number(currentMinTime)).append("_").append(QString::number(currentMaxTime)).append(".strc"));
                if(partTrace.open(QIODevice::ReadWrite|QIODevice::Text))
                {

                    int currentProc;
                    partTrace.write((QString(PROC_PER_FILE*10,QChar::Space)+"\n").toUtf8());
                    while(!trace.atEnd())
                    {
                        if(!isNextFile)
                        {
                            line = trace.readLine();
                            traceLine = line.split(' ');
                            int currentTime = traceLine.at(0).toInt();
                            if(currentTime>currentMaxTime)
                            {
                                isNextFile=true;
                                for(int i = 0; i<procs.length(); i++)
                                {
                                    if(conditions[i]==Conditions::active)
                                    {
                                        for (int j=solves[i]-currentMinTime;j<procs[i].length();j++) procs[i][j]=Conditions::active;
                                        sequentalTime[i]+=currentMaxTime-solves[i];
                                        solves[i]=currentMaxTime;

                                    }
                                    if(conditions[i]==Conditions::sending)
                                    {
                                        for (int j=sends[i]-currentMinTime;j<procs[i].length();j++) procs[i][j]=Conditions::sending;
                                        sends[i]=currentMaxTime;
                                    }
                                    if(conditions[i]==Conditions::receiving)
                                    {
                                        for (int j=recvs[i]-currentMinTime;j<procs[i].length();j++) procs[i][j]=Conditions::receiving;
                                        recvs[i]=currentMaxTime;
                                    }
                                }
                                break;
                            }
                        }
                        else
                        {
                            isNextFile =false;
                            int currentTime = traceLine.at(0).toInt();
                            if(currentTime>currentMaxTime)
                            {
                                isNextFile=true;
                                for(int i = 0; i<procs.length(); i++)
                                {
                                    if(conditions[i]==Conditions::active)
                                    {
                                        for (int j=solves[i]-currentMinTime;j<procs[i].length();j++) procs[i][j]=Conditions::active;
                                        sequentalTime[i]+=currentMaxTime-solves[i];
                                        solves[i]=currentMaxTime;

                                    }
                                    if(conditions[i]==Conditions::sending)
                                    {
                                        for (int j=sends[i]-currentMinTime;j<procs[i].length();j++) procs[i][j]=Conditions::sending;
                                        sends[i]=currentMaxTime;
                                    }
                                    if(conditions[i]==Conditions::receiving)
                                    {
                                        for (int j=recvs[i]-currentMinTime;j<procs[i].length();j++) procs[i][j]=Conditions::receiving;
                                        recvs[i]=currentMaxTime;
                                    }
                                }
                                break;
                            }
                        }

                        currentProc=traceLine.at(1).toInt();
                        if(currentProc>=currentMinProc && currentProc<=currentMaxProc)
                        {
                            //reading events
                            if(traceLine.length()>7)
                            {
                                if(traceLine.at(7).toInt()==BNBScheduler::Events::DONE)
                                {
                                    int done = traceLine.at(0).toInt();
                                    if(done<=currentMaxTime)
                                    {
                                        conditions[currentProc-currentMinProc]=Conditions::noaction;
                                        for (int i=solves[currentProc-currentMinProc]-currentMinTime;i<done-currentMinTime;i++) procs[currentProc-currentMinProc][i]=Conditions::active;
                                        sequentalTime[currentProc-currentMinProc]+=done - solves[currentProc-currentMinProc];
                                    }
                                }
                                else if(traceLine.at(7).toInt()==BNBScheduler::Events::SENT)
                                {
                                    int sent = traceLine.at(0).toInt();
                                    if(sent<=currentMaxTime)
                                    {
                                        conditions[currentProc-currentMinProc]=Conditions::noaction;
                                        for (int i=sends[currentProc-currentMinProc]-currentMinTime;i<=sent-currentMinTime;i++) procs[currentProc-currentMinProc][i]=Conditions::sending;
                                    }
                                }
                                else if(traceLine.at(7).toInt()==BNBScheduler::Events::COMMAND_ARRIVED ||
                                        traceLine.at(7).toInt()==BNBScheduler::Events::DATA_ARRIVED)
                                {
                                    int arrive = traceLine.at(0).toInt();
                                    if(arrive<=currentMaxTime)
                                    {
                                        conditions[currentProc-currentMinProc]=Conditions::noaction;
                                        for (int i=recvs[currentProc-currentMinProc]-currentMinTime;i<arrive-currentMinTime;i++) procs[currentProc-currentMinProc][i]=Conditions::receiving;
                                    }
                                }
                            }
                            //reading actions
                            if(traceLine.length()>2)
                            {
                                if(traceLine.at(2).toInt()==BNBScheduler::Actions::SOLVE)
                                {
                                    solves[currentProc-currentMinProc]=traceLine.at(0).toInt();
                                    conditions[currentProc-currentMinProc]=Conditions::active;
                                }
                                else if(traceLine.at(2).toInt()==BNBScheduler::Actions::SEND_COMMAND ||
                                        traceLine.at(2).toInt()==BNBScheduler::Actions::SEND_RECORDS ||
                                        traceLine.at(2).toInt()==BNBScheduler::Actions::SEND_SUB ||
                                        traceLine.at(2).toInt()==BNBScheduler::Actions::SEND_SUB_AND_RECORDS)
                                {
                                    sends[currentProc-currentMinProc]=traceLine.at(0).toInt();
                                    conditions[currentProc-currentMinProc]=Conditions::sending;
                                }
                                else if(traceLine.at(2).toInt()==BNBScheduler::Actions::RECV)
                                {
                                    recvs[currentProc-currentMinProc]=traceLine.at(0).toInt();
                                    conditions[currentProc-currentMinProc]=Conditions::receiving;
                                }
                            }
                        }
                    }
                    if(!statOnly)
                    {
                        for (int i = 0; i<=currentMaxTime-currentMinTime; i++)
                        {
                            partTrace.write(QString::number(i+currentMinTime).toUtf8());
                            for (int j = 0; j<=currentMaxProc-currentMinProc;j++)
                            {
                                //if(i==0)sequentalTime[j]=0;
                                partTrace.write(QString(" ").append(QString::number(procs[j][i])).toUtf8());
                                //if(procs[j][i]==Conditions::active)(sequentalTime[j])++;
                            }
                            partTrace.write(QString("\n").toUtf8());
                        }
                    }
                    partTrace.close();
                    for (int i =0;i<procs.length();i++) procs[i][0]=procs[i][currentMaxTime-currentMinTime];
                }
                if(partTrace.open(QIODevice::ReadWrite|QIODevice::Text))
                {
                    partTrace.write(QString("sequental:").toUtf8());
                    for (int j = 0; j<sequentalTime.length();j++)
                    {
                        partTrace.write(QString(" ").append(QString::number(sequentalTime[j])).toUtf8());
                    }
                    partTrace.close();
                }

            }//while(currentMaxTime<maxTime);
            trace.close();
            currentMinProc=currentMaxProc+1;
        }        
    }//while(currentMaxProc<procNum-1);
    return 0;
}

int TraceManager::countStatistics(double &speedup, double &efficiency, int procNum, int maxTime)
{
    QDir dir;
    if(!dir.exists("traces")){dir.mkdir("traces");return 1;}
    if(!dir.exists("traces/subtraces")){dir.mkdir("traces/subtraces");return 2;}
    dir.setPath("traces/subtraces");
    QStringList lst = dir.entryList(QDir::Files);
    QString line;
    QStringList partTraceLine;
    double sequentalTime = 0;
    if(lst.count()>0)
    {
        for (int i = 0; i<lst.count();i++)
        {
            QFile partTrace(dir.absoluteFilePath(lst[i]));
            if(partTrace.open(QIODevice::ReadOnly|QIODevice::Text))
            {
                line=partTrace.readLine();
                partTraceLine = line.split(' ');
                for (int j=1;j<partTraceLine.length();j++)
                {
                    sequentalTime += partTraceLine[j].toDouble();
                }
                speedup = sequentalTime/(double)maxTime;
                efficiency = speedup/(double)procNum;
                partTrace.close();
            }
            else return 4;
        }
    }
    else return 3;
    return 0;
}

