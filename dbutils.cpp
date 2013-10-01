#include <QFile>
#include <QTextStream>
#include <QtXml>
#include <QMessageBox>
#include "dbutils.h"
#include "settings.h"
#include "beaker.h"
#include "detector.h"

bool readSettingsXml(QFile &file, Settings& settings)
{
    QDomDocument document;
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Unable to open file: " + file.fileName());
        msgBox.exec();
        return false;
    }
    document.setContent(&file);
    file.close();

    QDomElement xroot = document.documentElement();
    if(xroot.tagName() != "Settings")
    {
        QMessageBox msgBox;
        msgBox.setText("Invalid root element: " + file.fileName());
        msgBox.exec();
        return false;
    }

    QDomNodeList xnodes = xroot.childNodes();
    for(int i=0; i<xnodes.count(); i++)
    {
        QDomElement xnode = xnodes.at(i).toElement();
        if(xnode.tagName() == "GenieFolder")
            settings.genieFolder = xnode.text();        
        else if(xnode.tagName() == "NIDLibrary")
            settings.NIDLibrary = xnode.text();
        else if(xnode.tagName() == "NIDConfidenceTreshold")
            settings.NIDConfidenceTreshold = xnode.text().toDouble();
        else if(xnode.tagName() == "NIDConfidenceFactor")
            settings.NIDConfidenceFactor = xnode.text().toDouble();
        else if(xnode.tagName() == "PerformMDATest")
            settings.performMDATest = (xnode.text() == "true") ? true : false;
        else if(xnode.tagName() == "InhibitATDCorrection")
            settings.inhibitATDCorrection = (xnode.text() == "true") ? true : false;
        else if(xnode.tagName() == "UseStoredLibrary")
            settings.useStoredLibrary = (xnode.text() == "true") ? true : false;
        else if(xnode.tagName() == "TemplateName")
            settings.templateName = xnode.text();
        else if(xnode.tagName() == "SectionName")
            settings.sectionName = xnode.text();
        else if(xnode.tagName() == "ErrorMultiplier")
            settings.errorMultiplier = xnode.text().toDouble();
    }
    return true;
}

bool writeSettingsXml(QFile &file, const Settings& settings)
{
    QDomDocument document;
    QDomElement xroot = document.createElement("Settings");
    document.appendChild(xroot);

    QDomElement xnode = document.createElement("GenieFolder");
    xnode.appendChild(document.createTextNode(settings.genieFolder));
    xroot.appendChild(xnode);    

    xnode = document.createElement("NIDLibrary");
    xnode.appendChild(document.createTextNode(settings.NIDLibrary));
    xroot.appendChild(xnode);

    xnode = document.createElement("NIDConfidenceTreshold");
    xnode.appendChild(document.createTextNode(QString::number(settings.NIDConfidenceTreshold)));
    xroot.appendChild(xnode);

    xnode = document.createElement("NIDConfidenceFactor");
    xnode.appendChild(document.createTextNode(QString::number(settings.NIDConfidenceFactor)));
    xroot.appendChild(xnode);

    xnode = document.createElement("PerformMDATest");
    xnode.appendChild(document.createTextNode(settings.performMDATest ? "true" : "false"));
    xroot.appendChild(xnode);

    xnode = document.createElement("InhibitATDCorrection");
    xnode.appendChild(document.createTextNode(settings.inhibitATDCorrection ? "true" : "false"));
    xroot.appendChild(xnode);

    xnode = document.createElement("UseStoredLibrary");
    xnode.appendChild(document.createTextNode(settings.useStoredLibrary ? "true" : "false"));
    xroot.appendChild(xnode);

    xnode = document.createElement("TemplateName");
    xnode.appendChild(document.createTextNode(settings.templateName));
    xroot.appendChild(xnode);

    xnode = document.createElement("SectionName");
    xnode.appendChild(document.createTextNode(settings.sectionName));
    xroot.appendChild(xnode);

    xnode = document.createElement("ErrorMultiplier");
    xnode.appendChild(document.createTextNode(QString::number(settings.errorMultiplier)));
    xroot.appendChild(xnode);

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Unable to open file: " + file.fileName());
        msgBox.exec();
        return false;
    }
    QTextStream stream(&file);
    stream << document.toString();
    file.close();
    return true;
}

bool readBeakerXml(QFile& file, QList<Beaker>& beakers)
{
    QDomDocument document;    
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Unable to open file: " + file.fileName());
        msgBox.exec();
        return false;
    }
    document.setContent(&file);
    file.close();

    beakers.clear();

    QDomElement xroot = document.firstChildElement();
    QDomNodeList xbeakers = xroot.elementsByTagName("Beaker");
    for(int i=0; i<xbeakers.count(); i++)
    {
        Beaker beaker;
        QDomElement xbeaker = xbeakers.at(i).toElement();

        beaker.name = xbeaker.attribute("Name");
        beaker.manufacturer = xbeaker.attribute("Manufacturer");
        beaker.enabled = xbeaker.attribute("Enabled") == "true" ? true : false;
        beakers.push_back(beaker);
    }
    return true;
}

bool writeBeakerXml(QFile& file, const QList<Beaker>& beakers)
{
    QDomDocument document;
    QDomElement xroot = document.createElement("Beakers");
    document.appendChild(xroot);
    for(int i=0; i<beakers.count(); i++)
    {
        QDomElement xbeaker = document.createElement("Beaker");
        xbeaker.setAttribute("Name", beakers[i].name);
        xbeaker.setAttribute("Manufacturer", beakers[i].manufacturer);
        xbeaker.setAttribute("Enabled", beakers[i].enabled ? "true" : "false");
        xroot.appendChild(xbeaker);
    }

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Unable to open file: " + file.fileName());
        msgBox.exec();
        return false;
    }
    QTextStream stream(&file);
    stream << document.toString();
    file.close();
    return true;
}

bool readDetectorXml(QFile& file, QList<Detector>& detectors)
{
    QDomDocument document;    
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Unable to open file: " + file.fileName());
        msgBox.exec();
        return false;
    }
    document.setContent(&file);
    file.close();

    detectors.clear();

    QDomElement xroot = document.firstChildElement();
    QDomNodeList xdetectors = xroot.elementsByTagName("Detector");
    for(int i=0; i<xdetectors.count(); i++)
    {
        Detector detector;
        QDomElement xdetector = xdetectors.at(i).toElement();

        detector.name = xdetector.attribute("Name");
        detector.enabled = xdetector.attribute("Enabled") == "true" ? true : false;
        detector.inUse = xdetector.attribute("InUse") == "true" ? true : false;
        detector.searchRegion = xdetector.attribute("SearchRegion").toInt();
        detector.significanceTreshold = xdetector.attribute("SignificanceTreshold").toDouble();
        detector.tolerance = xdetector.attribute("Tolerance").toDouble();
        detector.peakAreaRegion = xdetector.attribute("PeakAreaRegion").toInt();
        detector.continuum = xdetector.attribute("Continuum").toDouble();
        detector.continuumFunction = xdetector.attribute("ContinuumFunction");
        detector.criticalLevelTest = xdetector.attribute("CriticalLevelTest") == "true" ? true : false;
        detector.useFixedFWHM = xdetector.attribute("UseFixedFWHM") == "true" ? true : false;
        detector.useFixedTailParameter = xdetector.attribute("UseFixedTailParameter") == "true" ? true : false;
        detector.fitSinglets = xdetector.attribute("FitSingles") == "true" ? true : false;
        detector.displayROIs = xdetector.attribute("DisplayROIs") == "true" ? true : false;
        detector.rejectZeroAreaPeaks = xdetector.attribute("RejectZeroAreaPeaks") == "true" ? true : false;
        detector.maxFWHMsBetweenPeaks = xdetector.attribute("MaxFWHMsBetweenPeaks").toDouble();
        detector.maxFWHMsForLeftLimit = xdetector.attribute("MaxFWHMsForLeftLimit").toDouble();
        detector.maxFWHMsForRightLimit = xdetector.attribute("MaxFWHMsForRightLimit").toDouble();
        detector.backgroundSubtract = xdetector.attribute("BackgroundSubtract");
        detector.efficiencyCalibrationType = xdetector.attribute("EfficiencyCalibrationType");
        detector.presetType1 = xdetector.attribute("PresetType1");
        detector.presetType1Value = xdetector.attribute("PresetType1Value").toDouble();
        detector.presetType2 = xdetector.attribute("PresetType2");
        detector.presetType2Value = xdetector.attribute("PresetType2Value").toDouble();
        detector.spectrumCounter = xdetector.attribute("SpectrumCounter").toInt();
        detector.defaultBeaker = xdetector.attribute("DefaultBeaker");

        QDomNodeList xbeakers = xdetector.elementsByTagName("Beaker");
        for(int j=0; j<xbeakers.count(); j++)
        {
            QDomElement xbeaker = xbeakers.at(j).toElement();
            detector.beakers[xbeaker.attribute("BeakerName")] = xbeaker.attribute("CalFile");
        }

        detectors.push_back(detector);
    }
    return true;
}

bool writeDetectorXml(QFile& file, const QList<Detector>& detectors)
{
    QDomDocument document;
    QDomElement xroot = document.createElement("Detectors");
    document.appendChild(xroot);
    for(int i=0; i<detectors.count(); i++)
    {
        QDomElement xdetector = document.createElement("Detector");

        xdetector.setAttribute("Name", detectors[i].name);
        xdetector.setAttribute("Enabled", detectors[i].enabled ? "true" : "false");
        xdetector.setAttribute("InUse", detectors[i].inUse ? "true" : "false");
        xdetector.setAttribute("SearchRegion", detectors[i].searchRegion);
        xdetector.setAttribute("SignificanceTreshold", detectors[i].significanceTreshold);
        xdetector.setAttribute("Tolerance", detectors[i].tolerance);
        xdetector.setAttribute("PeakAreaRegion", detectors[i].peakAreaRegion);
        xdetector.setAttribute("Continuum", detectors[i].continuum);
        xdetector.setAttribute("ContinuumFunction", detectors[i].continuumFunction);
        xdetector.setAttribute("CriticalLevelTest", detectors[i].criticalLevelTest ? "true" : "false");
        xdetector.setAttribute("UseFixedFWHM", detectors[i].useFixedFWHM ? "true" : "false");
        xdetector.setAttribute("UseFixedTailParameter", detectors[i].useFixedTailParameter ? "true" : "false");
        xdetector.setAttribute("FitSingles", detectors[i].fitSinglets ? "true" : "false");
        xdetector.setAttribute("DisplayROIs", detectors[i].displayROIs ? "true" : "false");
        xdetector.setAttribute("RejectZeroAreaPeaks", detectors[i].rejectZeroAreaPeaks ? "true" : "false");
        xdetector.setAttribute("MaxFWHMsBetweenPeaks", detectors[i].maxFWHMsBetweenPeaks);
        xdetector.setAttribute("MaxFWHMsForLeftLimit", detectors[i].maxFWHMsForLeftLimit);
        xdetector.setAttribute("MaxFWHMsForRightLimit", detectors[i].maxFWHMsForRightLimit);
        xdetector.setAttribute("BackgroundSubtract", detectors[i].backgroundSubtract);
        xdetector.setAttribute("EfficiencyCalibrationType", detectors[i].efficiencyCalibrationType);
        xdetector.setAttribute("PresetType1", detectors[i].presetType1);
        xdetector.setAttribute("PresetType1Value", detectors[i].presetType1Value);
        xdetector.setAttribute("PresetType2", detectors[i].presetType2);
        xdetector.setAttribute("PresetType2Value", detectors[i].presetType2Value);
        xdetector.setAttribute("SpectrumCounter", detectors[i].spectrumCounter);
        xdetector.setAttribute("DefaultBeaker", detectors[i].defaultBeaker);

        QMapIterator<QString, QString> iter(detectors[i].beakers);
        while (iter.hasNext())
        {
            iter.next();
            QDomElement xbeaker = document.createElement("Beaker");
            xbeaker.setAttribute("BeakerName", iter.key());
            xbeaker.setAttribute("CalFile", iter.value());
            xdetector.appendChild(xbeaker);
        }

        xroot.appendChild(xdetector);
    }

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Unable to open file: " + file.fileName());
        msgBox.exec();
        return false;
    }
    QTextStream stream(&file);
    stream << document.toString();
    file.close();
    return true;
}


