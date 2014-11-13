#include "gui/dialogs/preferences_dialog.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QEvent>

#include "gui/app_style.h"
#include "gui/gui_factory.h"
#include "translations/translations.h"
#include "core/settings_manager.h"
#include "core/servers_manager.h"
#include "common/qt/convert_string.h"

namespace fastoredis
{
    PreferencesDialog::PreferencesDialog(QWidget *parent)
        : QDialog(parent)
    {
        setWindowIcon(GuiFactory::instance().mainWindowIcon());
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
        setFixedSize(height, width);

//      ui settings
        uiBox_ = new QGroupBox;

        QHBoxLayout *styleswLayout = new QHBoxLayout;
        stylesLabel_ = new QLabel;
        stylesComboBox_ = new QComboBox;
        stylesComboBox_->addItems(getSupportedStyles());
        styleswLayout->addWidget(stylesLabel_);
        styleswLayout->addWidget(stylesComboBox_);

        QHBoxLayout *langLayout = new QHBoxLayout;
        langLabel_ = new QLabel;
        langLayout->addWidget(langLabel_);
        languagesComboBox_  = new QComboBox;
        languagesComboBox_->addItems(translations::getSupportedLanguages());
        langLayout->addWidget(languagesComboBox_);

        QVBoxLayout *uiLayout = new QVBoxLayout;
        uiLayout->addLayout(styleswLayout);
        uiLayout->addLayout(langLayout);
        uiBox_->setLayout(uiLayout);

//      servers settings
        serverSettingsBox_ = new QGroupBox;

        QHBoxLayout* defaultViewLayaut = new QHBoxLayout;
        defaultViewLabel_ = new QLabel;
        defaultViewComboBox_ = new QComboBox;
        std::vector<std::string> allV = allSupportedViews();
        for(int i = 0; i < allV.size(); ++i){
            defaultViewComboBox_->addItem(common::convertFromString<QString>(allV[i]));
        }
        defaultViewLayaut->addWidget(defaultViewLabel_);
        defaultViewLayaut->addWidget(defaultViewComboBox_);

        syncTabs_ = new QCheckBox;
        logDirPath_ = new QLineEdit;
        QHBoxLayout *logLayout = new QHBoxLayout;
        logDirLabel_ = new QLabel;
        logLayout->addWidget(logDirLabel_);
        logLayout->addWidget(logDirPath_);

        QVBoxLayout *serverSettingsLayout = new QVBoxLayout;
        serverSettingsLayout->addLayout(defaultViewLayaut);
        serverSettingsLayout->addWidget(syncTabs_);
        serverSettingsLayout->addLayout(logLayout);
        serverSettingsBox_->setLayout(serverSettingsLayout);

//      main layout
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(uiBox_);
        layout->addWidget(serverSettingsBox_);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
        VERIFY(connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept())));
        VERIFY(connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject())));
        layout->addWidget(buttonBox);
        setLayout(layout);

        syncWithSettings();
        retranslateUi();
    }

    void PreferencesDialog::syncWithSettings()
    {
        languagesComboBox_->setCurrentText(common::convertFromString<QString>(SettingsManager::instance().currentLanguage()));
        stylesComboBox_->setCurrentText(common::convertFromString<QString>(SettingsManager::instance().currentStyle()));
        defaultViewComboBox_->setCurrentText(common::convertFromString<QString>(common::convertToString(SettingsManager::instance().defaultView())));
        syncTabs_->setChecked(SettingsManager::instance().syncTabs());
        logDirPath_->setText(common::convertFromString<QString>(SettingsManager::instance().loggingDirectory()));
    }

    void PreferencesDialog::accept()
    {
        QString newLang = translations::applyLanguage(languagesComboBox_->currentText());
        SettingsManager::instance().setCurrentLanguage(common::convertToString(newLang));

        applyStyle(stylesComboBox_->currentText());
        SettingsManager::instance().setCurrentStyle(common::convertToString(stylesComboBox_->currentText()));

        const std::string defCombo = common::convertToString(defaultViewComboBox_->currentText());
        const fastoredis::supportedViews v = common::convertFromString<fastoredis::supportedViews>(defCombo);
        SettingsManager::instance().setDefaultView(v);

        ServersManager::instance().setSyncServers(syncTabs_->isChecked());
        SettingsManager::instance().setSyncTabs(syncTabs_->isChecked());

        return QDialog::accept();
    }

    void PreferencesDialog::changeEvent(QEvent *e)
    {
        if(e->type() == QEvent::LanguageChange){
            retranslateUi();
        }
        QDialog::changeEvent(e);
    }

    void PreferencesDialog::retranslateUi()
    {
        setWindowTitle(tr("Preferences "PROJECT_NAME_TITLE));

        uiBox_->setTitle(tr("User interface"));
        langLabel_->setText(tr("Language:"));
        stylesLabel_->setText(tr("Supported UI styles:"));

        serverSettingsBox_->setTitle(tr("Servers global settings"));
        defaultViewLabel_->setText(tr("Default views:"));
        syncTabs_->setText(tr("Sync tabs"));
        logDirLabel_->setText(tr("Logging directory:"));
    }
}