#include "dspextractsignalplugin.h"
#include "pluginmanager.h"
#include "pluginconnectorqueued.h"

#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>

static PluginRegistrar registrar ("dspextractsignal", DspExtractSignalPlugin::create, AbstractPlugin::GroupDSP);

DspExtractSignalPlugin::DspExtractSignalPlugin(int _id, QString _name)
    : BasePlugin(_id, _name)
{
    createSettings(settingsLayout);

    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::in,"trigger"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::in,"signal"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::out,"shape vector"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::out,"baseline (v/c)"));

    std::cout << "Instantiated DspExtractSignalPlugin" << std::endl;
}

void DspExtractSignalPlugin::createSettings(QGridLayout * l)
{
    // Plugin specific code here

    QWidget* container = new QWidget();
    {
        QGridLayout* cl = new QGridLayout;

        QLabel* label = new QLabel(tr("This plugin extracts signals based on a trigger mask from a long trace."));
        QLabel* wlabel = new QLabel(tr("Width"));
        QLabel* olabel = new QLabel(tr("Offset"));

        widthSpinner = new QSpinBox();
        offsetSpinner = new QSpinBox();
        invertBox = new QCheckBox(tr("Invert"));

        widthSpinner->setValue(50);
        offsetSpinner->setValue(5);

        widthSpinner->setMaximum(1000000);
        offsetSpinner->setMaximum(1000000);

        invertBox->setChecked(true);

        connect(widthSpinner,SIGNAL(valueChanged(int)),this,SLOT(widthChanged()));
        connect(offsetSpinner,SIGNAL(valueChanged(int)),this,SLOT(offsetChanged()));
        connect(invertBox,SIGNAL(toggled(bool)),this,SLOT(invertChanged(bool)));

        cl->addWidget(label,0,0,1,2);
        cl->addWidget(wlabel,1,0,1,1);
        cl->addWidget(olabel,2,0,1,1);
        cl->addWidget(widthSpinner,1,1,1,1);
        cl->addWidget(offsetSpinner,2,1,1,1);
        cl->addWidget(invertBox,3,0,1,1);

        container->setLayout(cl);
    }

    // End

    l->addWidget(container,0,0,1,1);
}

void DspExtractSignalPlugin::widthChanged()
{
    int newValue = widthSpinner->value();
    this->conf.width = newValue;
}

void DspExtractSignalPlugin::offsetChanged()
{
    int newValue = offsetSpinner->value();
    this->conf.offset = newValue;
}

void DspExtractSignalPlugin::invertChanged(bool newValue)
{
    this->conf.invert = newValue;
}

void DspExtractSignalPlugin::applySettings(QSettings* settings)
{
    QString set;
    settings->beginGroup(getName());
        set = "width";   if(settings->contains(set)) conf.width = settings->value(set).toInt();
        set = "offset";   if(settings->contains(set)) conf.offset = settings->value(set).toInt();
        set = "invert";   if(settings->contains(set)) conf.invert = settings->value(set).toBool();
    settings->endGroup();

    // UI update
    widthSpinner->setValue(conf.width);
    offsetSpinner->setValue(conf.offset);
    invertBox->setChecked(conf.invert);
}

void DspExtractSignalPlugin::saveSettings(QSettings* settings)
{
    if(settings == NULL)
    {
        std::cout << getName().toStdString() << ": no settings file" << std::endl;
        return;
    }
    else
    {
        std::cout << getName().toStdString() << " saving settings...";
        settings->beginGroup(getName());
            settings->setValue("width",conf.width);
            settings->setValue("offset",conf.offset);
            settings->setValue("invert",conf.invert);
        settings->endGroup();
        std::cout << " done" << std::endl;
    }
}

void DspExtractSignalPlugin::userProcess()
{
    //std::cout << "DspExtractSignalPlugin Processing" << std::endl;
    const vector<double>* ptrigger = reinterpret_cast<const std::vector<double>*>(inputs->at(0)->getData());
    const vector<double>* pdata = reinterpret_cast<const std::vector<double>*>(inputs->at(1)->getData());

    SamDSP dsp;

    // Convert to double
    vector<double> data((*pdata).begin(),(*pdata).end());
    //vector<double> trigger((*ptrigger).begin(),(*ptrigger).end());
    vector<double> baseline_mask(ptrigger->size(),1);
    vector<double> allowedTrigger(ptrigger->size(),0);

    baseline_out.resize(2,0);
    signal.resize(conf.width,0);

    // Fill baseline_mask and allowedTrigger
    for(int i = 0; i < (int)ptrigger->size(); i++)
    {
        if(ptrigger->at(i) == 1)
        {
            allowedTrigger.at(i) = 1;
            for(int j = -conf.offset; j < conf.width-conf.offset; j++)
            {
                if(i+j > 0 && i+j < (int)baseline_mask.size())
                {
                    baseline_mask.at(i+j) = 0;
                }
            }
            for(int j = -conf.width; j < conf.width; j++)
            {
                if(i+j > 0 && i+j < (int)baseline_mask.size())
                {
                    if(j != 0) allowedTrigger.at(i+j) = 0;
                }
            }
        }
    }


    // Calculate baseline value
    int cnt = 0;
    double baseline = 0;
    for(int i = 0; i < (int)data.size(); i++)
    {
        if(baseline_mask.at(i) == 1)
        {
            baseline += data.at(i);
            cnt++;
        }
    }
    baseline /= (double)(cnt);

//    std::cout << "Calculated baseline: " << baseline << " from " << cnt << " points." << std::endl; std::flush(std::cout);

    // Extract signal
    dsp.fast_addC(data,-baseline);

    signal = dsp.average(data,allowedTrigger,(unsigned int)conf.offset,(unsigned int)(conf.width-conf.offset));

    // Invert
    if(conf.invert)
    {
        dsp.fast_scale(signal,-1.0);
        baseline *= -1.0;
    }

    baseline_out[0] = baseline;
    baseline_out[1] = cnt;

    outputs->first()->setData(&signal);
    outputs->at(1)->setData(&baseline_out);
}
