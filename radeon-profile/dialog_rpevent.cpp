#include "dialog_rpevent.h"
#include "ui_dialog_rpevent.h"
#include "radeon_profile.h"
#include "globalStuff.h"

#include <QLineEdit>
#include <QMap>

Dialog_RPEvent::Dialog_RPEvent(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_RPEvent)
{
    ui->setupUi(this);
    setFixedSize(size());

    setFixedFanSpeedVisibility(false);
    ui->spin_fixedFanSpeed->setMinimum(radeon_profile::minFanStepsSpeed);
}

void Dialog_RPEvent::setFeatures(const globalStuff::driverFeatures &features, const QList<QString> &profiles) {
    switch (features.pm) {
        case globalStuff::powerMethod::DPM:
            ui->combo_dpmChange->addItems(globalStuff::createDPMCombo());
            ui->combo_powerLevelChange->addItems(globalStuff::createPowerLevelCombo());
            break;
        case globalStuff::powerMethod::PROFILE:
            ui->combo_dpmChange->addItems(globalStuff::createProfileCombo());
            ui->combo_powerLevelChange->setVisible(false);
            ui->l_powerLevel->setVisible(false);

            ui->l_profile->setText(tr("Set Profile to:"));
            break;
        default:
            break;
    }

    ui->combo_fanChange->addItem(tr("Auto"));
    ui->combo_fanChange->addItem(tr("Fixed speed"));

    for (QString p : profiles)
        ui->combo_fanChange->addItem(p);

    if (!features.pwmAvailable) {
        ui->combo_fanChange->setVisible(false);
        ui->l_fan->setVisible(false);
    }
}

Dialog_RPEvent::~Dialog_RPEvent()
{
    delete ui;
}

void Dialog_RPEvent::on_btn_cancel_clicked()
{
    this->reject();
}

void Dialog_RPEvent::on_btn_save_clicked()
{
    if (ui->edt_eventName->text().isEmpty())
        return;

    this->setResult(QDialog::Accepted);

    switch (ui->combo_eventTrigger->currentIndex()) {
        case 0:
            createdEvent.type = rpeventType::TEMPEREATURE;
            break;
        case 1:
            createdEvent.type = rpeventType::BIANRY;

            if (ui->edt_binary->text().isEmpty()) {
                QMessageBox::information(this, "", tr("Selected trigger type is Binary, so the binary field cannot be empty."),QMessageBox::Ok);
                return;
            }
            break;
    }

    createdEvent.enabled = ui->cb_enabled->isChecked();
    createdEvent.name = ui->edt_eventName->text();

    createdEvent.activationBinary = ui->edt_binary->text();
    createdEvent.activationTemperature = ui->spin_tempActivate->value();

    createdEvent.dpmProfileChange = createdEvent.getEnumFromCombo<globalStuff::powerProfiles>(ui->combo_dpmChange->currentIndex());
    createdEvent.powerLevelChange = createdEvent.getEnumFromCombo<globalStuff::forcePowerLevels>(ui->combo_powerLevelChange->currentIndex());

    createdEvent.fixedFanSpeedChange  = ui->spin_fixedFanSpeed->value();
    createdEvent.fanComboIndex = ui->combo_fanChange->currentIndex();

    if (ui->combo_fanChange->currentIndex() > 1)
        createdEvent.fanProfileNameChange = ui->combo_fanChange->currentText();

    this->accept();
}

RPEvent Dialog_RPEvent::getCreatedEvent() {
    return createdEvent;
}

void Dialog_RPEvent::setEditedEvent(const RPEvent &rpe) {
    createdEvent = rpe;

    ui->combo_eventTrigger->setCurrentIndex(rpe.type);
    ui->cb_enabled->setChecked(rpe.enabled);
    ui->edt_eventName->setText(rpe.name);
    ui->spin_tempActivate->setValue(rpe.activationTemperature);
    ui->edt_binary->setText(rpe.activationBinary);
    ui->combo_dpmChange->setCurrentIndex(rpe.dpmProfileChange + 1);
    ui->combo_powerLevelChange->setCurrentIndex(rpe.powerLevelChange + 1);

    if (rpe.fanComboIndex > 1)
        ui->combo_fanChange->setCurrentIndex(ui->combo_fanChange->findText(rpe.fanProfileNameChange));
    else
        ui->combo_fanChange->setCurrentIndex(rpe.fanComboIndex);

    ui->spin_fixedFanSpeed->setValue(rpe.fixedFanSpeedChange);
}

void Dialog_RPEvent::setFixedFanSpeedVisibility(bool visibility) {
    ui->widget_fixedFanSpeed->setVisible(visibility);
}

void Dialog_RPEvent::on_combo_fanChange_currentIndexChanged(int index)
{
    if (index == 2)
        setFixedFanSpeedVisibility(true);
     else
        setFixedFanSpeedVisibility(false);
}

void Dialog_RPEvent::on_btn_setBinary_clicked()
{
    QString binaryPath = QFileDialog::getOpenFileName(this, tr("Select binary"));

    if (!binaryPath.isEmpty())
        ui->edt_binary->setText(binaryPath);
}
