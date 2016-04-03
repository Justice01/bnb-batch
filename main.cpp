#include <iostream>
#include "tracemanager.h"
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QProcess>
#include <QDateTime>
int main(int argc, char *argv[])
{


    if (argc<3)
    {
        std::cout<< "Not enough parameters!" <<std::endl;
        return 1;
    }
    QFile::remove(argv[2]);
    QFile pars(argv[1]);
    QFile rslts(argv[2]);
    //getting maxTime and procNum
    if(pars.open(QIODevice::ReadOnly|QIODevice::Text) && rslts.open(QIODevice::ReadWrite|QIODevice::Text))
    {
        QString headline;
        QString line;
        QStringList parLine;
        QString solve,store,load,overhead,command,record,subproblem;
        headline = pars.readLine();
        headline[headline.length()-1]=' ';
        rslts.write(headline.append("Speedup Efficiency\n").toUtf8());
        while(!pars.atEnd())
        {
            line = pars.readLine();
            //std::cout<<line.toStdString();
            parLine = line.split(' ');

            QFile jsonFile("settings.json");
            if(jsonFile.open(QIODevice::ReadOnly))
            {
                QJsonObject obj = QJsonDocument::fromJson(((QString)jsonFile.readAll()).toUtf8()).object();
                solve = QString::number(((obj.value("timer").toObject()).value("time cost").toObject())["solve"].toDouble());
                store = QString::number(((obj.value("timer").toObject()).value("time cost").toObject())["store"].toDouble());
                load = QString::number(((obj.value("timer").toObject()).value("time cost").toObject())["load"].toDouble());
                overhead = QString::number(((obj.value("timer").toObject()).value("time cost").toObject())["overhead"].toDouble());
                command = QString::number(((obj.value("serializer").toObject()).value("parcel size").toObject())["command"].toDouble());
                record = QString::number(((obj.value("serializer").toObject()).value("parcel size").toObject())["record"].toDouble());
                subproblem = QString::number(((obj.value("serializer").toObject()).value("parcel size").toObject())["subproblem"].toDouble());
            }
            else
            {
                std::cout << "settings file was not load!" << std::endl;
                return 3;
            }
            jsonFile.close();
            if(jsonFile.open(QIODevice::ReadWrite|QIODevice::Truncate))
            {
                QJsonObject obj;

                QJsonObject timer;
                QJsonObject time_cost;

                QJsonObject serializer;
                QJsonObject parcel_size;

                QJsonObject communicator;

                QJsonObject resolver;

                time_cost["solve"]= solve.toInt();
                time_cost["store"]= store.toInt();
                time_cost["load"]= load.toInt();
                time_cost["overhead"]= overhead.toInt();

                timer["time cost"]=time_cost;

                parcel_size["command"]=command.toInt();
                parcel_size["record"]=record.toInt();
                parcel_size["subproblem"]=subproblem.toInt();

                serializer["parcel size"]=parcel_size;

                communicator["latency"]=parLine[2].toInt();
                communicator["bandwidth"]=parLine[3].toInt();

                resolver["maximal task level"]=parLine[4].toInt();
                resolver["use the same trees"]=parLine[0].toInt();
                resolver["seed"] = parLine[1].toInt();

                obj["timer"]=timer;
                obj["serializer"]=serializer;
                obj["communicator"]=communicator;
                obj["resolver"]=resolver;
                QJsonDocument doc(obj);
                jsonFile.write(doc.toJson());
            }
            else
            {
                std::cout << "settings file was not save!" << std::endl;
                return 3;
            }
            jsonFile.close();


            //reading bnb-simulators data (trace)
            if(!QFile::exists("bnbtest"))
            {
                std::cout<< "executable simulator bnbtest not found" <<std::endl;
                return 3;
            }
            QString s("traces/");
            QDateTime d = QDateTime::currentDateTime();
            int dateTimeItem=d.date().year();
            s.append(QString::number(dateTimeItem));
            dateTimeItem=d.date().month();
            s.append(dateTimeItem>10?QString::number(dateTimeItem):("0"+QString::number(dateTimeItem)));
            dateTimeItem=d.date().day();
            s.append(dateTimeItem>10?QString::number(dateTimeItem):("0"+QString::number(dateTimeItem)));
            dateTimeItem=d.time().hour();
            s.append(dateTimeItem>10?QString::number(dateTimeItem):("0"+QString::number(dateTimeItem)));
            dateTimeItem=d.time().minute();
            s.append(dateTimeItem>10?QString::number(dateTimeItem):("0"+QString::number(dateTimeItem)));
            dateTimeItem=d.time().second();
            s.append(dateTimeItem>10?QString::number(dateTimeItem):("0"+QString::number(dateTimeItem)));
            QProcess p;
            p.setStandardOutputFile(s.append(".trc"));
            p.setReadChannelMode(QProcess::MergedChannels);
            p.start("./bnbtest");
            QString processParams = QString(parLine[5]+" "+parLine[6]);
            if(argc>7)
            {
                for(int i =8; i<argc; i++) processParams.append(" ").append(argv[i]);
            }
            p.write(QString(parLine[5]+" "+parLine[6]).toLocal8Bit());
            p.closeWriteChannel();
            while(p.waitForStarted(-1))
            {

                while(p.waitForReadyRead(-1))
                {
                }
            }
            p.waitForFinished();
            static int c = 0;
            c++;
            std::cout << "line N " << c << " done!"<< std::endl;

            int maxTime=0;
            int procNum=0;
            TraceManager::parseTrace(s, procNum, maxTime,true);
            double speedup = 0.0;
            double efficiency = 0.0;
            TraceManager::countStatistics(speedup,efficiency,procNum,maxTime);
            line[line.length()-1] = ' ';

            rslts.write(line.append(QString::number(speedup)).append(" ").append(QString::number(efficiency)).append("\n").toUtf8());
        }

    }
    else
    {
        std::cout<<"Files couldn`t be open!"<<std::endl;
        return 2;
    }
    pars.close();
    rslts.close();
    std::cout << "all done!"<< std::endl;
}
