#include "gui/explorer/explorer_tree_view.h"

#include <QMenu>
#include <QMessageBox>
#include <QHeaderView>
#include <QAction>
#include <QFileDialog>

#include "gui/explorer/explorer_tree_model.h"

#include "gui/dialogs/info_server_dialog.h"
#include "gui/dialogs/property_server_dialog.h"
#include "gui/dialogs/history_server_dialog.h"
#include "gui/dialogs/load_contentdb_dialog.h"
#include "gui/dialogs/create_dbkey_dialog.h"

#include "common/qt/convert_string.h"

#include "translations/global.h"

namespace fastoredis
{
    ExplorerTreeView::ExplorerTreeView(QWidget* parent)
        : QTreeView(parent)
    {
        setModel(new ExplorerTreeModel(this));

        setSelectionBehavior(QAbstractItemView::SelectRows);
        setContextMenuPolicy(Qt::CustomContextMenu);
        VERIFY(connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&))));

        connectAction_ = new QAction(this);
        VERIFY(connect(connectAction_, &QAction::triggered, this, &ExplorerTreeView::connectDisconnectToServer));
        openConsoleAction_ = new QAction(this);
        VERIFY(connect(openConsoleAction_, &QAction::triggered, this, &ExplorerTreeView::openConsole));
        loadDatabaseAction_ = new QAction(this);
        VERIFY(connect(loadDatabaseAction_, &QAction::triggered, this, &ExplorerTreeView::loadDatabases));

        infoServerAction_ = new QAction(this);
        VERIFY(connect(infoServerAction_, &QAction::triggered, this, &ExplorerTreeView::openInfoServerDialog));

        propertyServerAction_ = new QAction(this);
        VERIFY(connect(propertyServerAction_, &QAction::triggered, this, &ExplorerTreeView::openPropertyServerDialog));

        historyServerAction_ = new QAction(this);
        VERIFY(connect(historyServerAction_, &QAction::triggered, this, &ExplorerTreeView::openHistoryServerDialog));

        closeAction_ = new QAction(this);
        VERIFY(connect(closeAction_, &QAction::triggered, this, &ExplorerTreeView::closeConnection));

        importAction_ = new QAction(this);
        VERIFY(connect(importAction_, &QAction::triggered, this, &ExplorerTreeView::importServer));

        backupAction_ = new QAction(this);
        VERIFY(connect(backupAction_, &QAction::triggered, this, &ExplorerTreeView::backupServer));

        shutdownAction_ = new QAction(this);
        VERIFY(connect(shutdownAction_, &QAction::triggered, this, &ExplorerTreeView::shutdownServer));

        loadContentAction_ = new QAction(this);
        VERIFY(connect(loadContentAction_, &QAction::triggered, this, &ExplorerTreeView::loadContentDb));

        setDefaultDbAction_ = new QAction(this);
        VERIFY(connect(setDefaultDbAction_, &QAction::triggered, this, &ExplorerTreeView::setDefaultDb));

        createKeyAction_ = new QAction(this);
        VERIFY(connect(createKeyAction_, &QAction::triggered, this, &ExplorerTreeView::createKey));

        getValueAction_ = new QAction(this);
        VERIFY(connect(getValueAction_, &QAction::triggered, this, &ExplorerTreeView::getValue));

        deleteKeyAction_ = new QAction(this);
        VERIFY(connect(deleteKeyAction_, &QAction::triggered, this, &ExplorerTreeView::deleteKey));

        retranslateUi();
    }

    void ExplorerTreeView::addServer(IServerSPtr server)
    {
        DCHECK(server);
        if(!server){
            return;
        }

        ExplorerTreeModel *mod = static_cast<ExplorerTreeModel*>(model());
        DCHECK(mod);
        if(!mod){
            return;
        }

        VERIFY(connect(server.get(), &IServer::startedLoadDatabases, this, &ExplorerTreeView::startLoadDatabases));
        VERIFY(connect(server.get(), &IServer::finishedLoadDatabases, this, &ExplorerTreeView::finishLoadDatabases));
        VERIFY(connect(server.get(), &IServer::startedSetDefaultDatabase, this, &ExplorerTreeView::startSetDefaultDatabase));
        VERIFY(connect(server.get(), &IServer::finishedSetDefaultDatabase, this, &ExplorerTreeView::finishSetDefaultDatabase));
        VERIFY(connect(server.get(), &IServer::startedLoadDataBaseContent, this, &ExplorerTreeView::startLoadDatabaseContent));
        VERIFY(connect(server.get(), &IServer::finishedLoadDatabaseContent, this, &ExplorerTreeView::finishLoadDatabaseContent));
        VERIFY(connect(server.get(), &IServer::startedExecuteCommand, this, &ExplorerTreeView::startExecuteCommand));
        VERIFY(connect(server.get(), &IServer::finishedExecuteCommand, this, &ExplorerTreeView::finishExecuteCommand));

        mod->addServer(server);
    }

    void ExplorerTreeView::removeServer(IServerSPtr server)
    {
        DCHECK(server);
        if(!server){
            return;
        }

        ExplorerTreeModel *mod = static_cast<ExplorerTreeModel*>(model());
        DCHECK(mod);
        if(!mod){
            return;
        }

        VERIFY(disconnect(server.get(), &IServer::startedLoadDatabases, this, &ExplorerTreeView::startLoadDatabases));
        VERIFY(disconnect(server.get(), &IServer::finishedLoadDatabases, this, &ExplorerTreeView::finishLoadDatabases));
        VERIFY(disconnect(server.get(), &IServer::startedSetDefaultDatabase, this, &ExplorerTreeView::startSetDefaultDatabase));
        VERIFY(disconnect(server.get(), &IServer::finishedSetDefaultDatabase, this, &ExplorerTreeView::finishSetDefaultDatabase));
        VERIFY(disconnect(server.get(), &IServer::startedLoadDataBaseContent, this, &ExplorerTreeView::startLoadDatabaseContent));
        VERIFY(disconnect(server.get(), &IServer::finishedLoadDatabaseContent, this, &ExplorerTreeView::finishLoadDatabaseContent));
        VERIFY(disconnect(server.get(), &IServer::startedExecuteCommand, this, &ExplorerTreeView::startExecuteCommand));
        VERIFY(disconnect(server.get(), &IServer::finishedExecuteCommand, this, &ExplorerTreeView::finishExecuteCommand));

        mod->removeServer(server);
        emit closeServer(server);
    }

    void ExplorerTreeView::showContextMenu(const QPoint& point)
    {
        QPoint menuPoint = mapToGlobal(point);
        menuPoint.setY(menuPoint.y() + header()->height());

        QModelIndex sel = selectedIndex();
        if(sel.isValid()){            
            IExplorerTreeItem *node = common::utils_qt::item<IExplorerTreeItem*>(sel);
            DCHECK(node);
            if(!node){
                return;
            }

            if(node->type() == IExplorerTreeItem::Server){
                QMenu menu(this);                
                menu.addAction(connectAction_);
                menu.addAction(openConsoleAction_);

                IServerSPtr server = node->server();
                bool isCon = server->isConnected();

                loadDatabaseAction_->setEnabled(isCon);
                menu.addAction(loadDatabaseAction_);
                infoServerAction_->setEnabled(isCon);
                menu.addAction(infoServerAction_);
                propertyServerAction_->setEnabled(isCon);
                menu.addAction(propertyServerAction_);

                menu.addAction(historyServerAction_);
                menu.addAction(closeAction_);

                bool isLocal = server->isLocalHost();

                importAction_->setEnabled(!isCon && isLocal);
                menu.addAction(importAction_);                
                backupAction_->setEnabled(isCon && isLocal);
                menu.addAction(backupAction_);
                shutdownAction_->setEnabled(isCon);
                menu.addAction(shutdownAction_);

                menu.exec(menuPoint);
            }
            else if(node->type() == IExplorerTreeItem::Database){
                ExplorerDatabaseItem *db = dynamic_cast<ExplorerDatabaseItem*>(node);
                QMenu menu(this);
                menu.addAction(loadContentAction_);
                bool isDefault = db && db->isDefault();
                loadContentAction_->setEnabled(isDefault);

                menu.addAction(createKeyAction_);
                createKeyAction_->setEnabled(isDefault);

                menu.addAction(setDefaultDbAction_);
                setDefaultDbAction_->setEnabled(!isDefault);
                menu.exec(menuPoint);
            }
            else if(node->type() == IExplorerTreeItem::Key){
                QMenu menu(this);
                menu.addAction(getValueAction_);
                menu.addAction(deleteKeyAction_);
                menu.exec(menuPoint);
            }
        }
    }

    void ExplorerTreeView::connectDisconnectToServer()
    {
        QModelIndex sel = selectedIndex();
        if(!sel.isValid()){
            return;
        }

        ExplorerServerItem *node = common::utils_qt::item<ExplorerServerItem*>(sel);
        if(!node){
            return;
        }

        IServerSPtr server = node->server();
        if(!server){
            return;
        }

        if(server->isConnected()){
            server->disconnect();
        }
        else{
            server->connect();
        }
    }

    void ExplorerTreeView::openConsole()
    {
        QModelIndex sel = selectedIndex();
        if(!sel.isValid()){
            return;
        }

        ExplorerServerItem *node = common::utils_qt::item<ExplorerServerItem*>(sel);
        if(node){
            emit openedConsole(node->server(), QString());
        }
    }

    void ExplorerTreeView::loadDatabases()
    {
        QModelIndex sel = selectedIndex();
        if(!sel.isValid()){
            return;
        }

        ExplorerServerItem *node = common::utils_qt::item<ExplorerServerItem*>(sel);
        if(node){
            node->loadDatabases();
        }
    }

    void ExplorerTreeView::openInfoServerDialog()
    {
        QModelIndex sel = selectedIndex();
        if(!sel.isValid()){
            return;
        }

        ExplorerServerItem *node = common::utils_qt::item<ExplorerServerItem*>(sel);
        if(!node){
            return;
        }

        IServerSPtr server = node->server();
        if(!server){
            return;
        }

        InfoServerDialog infDialog(QString("%1 info").arg(server->name()), server->type(), this);
        VERIFY(connect(server.get(), &IServer::startedLoadServerInfo, &infDialog, &InfoServerDialog::startServerInfo));
        VERIFY(connect(server.get(), &IServer::finishedLoadServerInfo, &infDialog, &InfoServerDialog::finishServerInfo));
        VERIFY(connect(&infDialog, &InfoServerDialog::showed, server.get(), &IServer::serverInfo));
        infDialog.exec();
    }

    void ExplorerTreeView::openPropertyServerDialog()
    {
        QModelIndex sel = selectedIndex();
        if(!sel.isValid()){
            return;
        }

        ExplorerServerItem *node = common::utils_qt::item<ExplorerServerItem*>(sel);
        if(!node){
            return;
        }

        IServerSPtr server = node->server();
        if(!server){
            return;
        }

        PropertyServerDialog infDialog(QString("%1 properties").arg(server->name()), server->type(), this);
        VERIFY(connect(server.get(), &IServer::startedLoadServerProperty, &infDialog, &PropertyServerDialog::startServerProperty));
        VERIFY(connect(server.get(), &IServer::finishedLoadServerProperty, &infDialog, &PropertyServerDialog::finishServerProperty));
        VERIFY(connect(server.get(), &IServer::startedChangeServerProperty, &infDialog, &PropertyServerDialog::startServerChangeProperty));
        VERIFY(connect(server.get(), &IServer::finishedChangeServerProperty, &infDialog, &PropertyServerDialog::finishServerChangeProperty));
        VERIFY(connect(&infDialog, &PropertyServerDialog::changedProperty, server.get(), &IServer::changeProperty));
        VERIFY(connect(&infDialog, &PropertyServerDialog::showed, server.get(), &IServer::serverProperty));
        infDialog.exec();
    }

    void ExplorerTreeView::openHistoryServerDialog()
    {
        QModelIndex sel = selectedIndex();
        if(!sel.isValid()){
            return;
        }

        ExplorerServerItem *node = common::utils_qt::item<ExplorerServerItem*>(sel);
        if(!node){
            return;
        }

        IServerSPtr server = node->server();
        if(!server){
            return;
        }

        ServerHistoryDialog histDialog(QString("%1 history").arg(server->name()), server->type(), this);
        VERIFY(connect(server.get(), &IServer::startedLoadServerHistoryInfo, &histDialog, &ServerHistoryDialog::startLoadServerHistoryInfo));
        VERIFY(connect(server.get(), &IServer::finishedLoadServerHistoryInfo, &histDialog, &ServerHistoryDialog::finishLoadServerHistoryInfo));
        VERIFY(connect(server.get(), &IServer::serverInfoSnapShoot, &histDialog, &ServerHistoryDialog::snapShotAdd));
        VERIFY(connect(&histDialog, &ServerHistoryDialog::showed, server.get(), &IServer::requestHistoryInfo));
        histDialog.exec();
    }

    void ExplorerTreeView::closeConnection()
    {
        QModelIndex sel = selectedIndex();
        if(!sel.isValid()){
            return;
        }

        ExplorerServerItem *node = common::utils_qt::item<ExplorerServerItem*>(sel);
        if(node){
            IServerSPtr server = node->server();
            if(server){
                removeServer(server);
            }
        }
    }

    void ExplorerTreeView::backupServer()
    {
        QModelIndex sel = selectedIndex();
        if(!sel.isValid()){
            return;
        }

        ExplorerServerItem *node = common::utils_qt::item<ExplorerServerItem*>(sel);
        if(!node){
            return;
        }

        IServerSPtr server = node->server();

        using namespace translations;
        QString filepath = QFileDialog::getOpenFileName(this, trBackup, QString(), trfilterForRdb);
        if (!filepath.isEmpty() && server) {
            server->backupToPath(filepath);
        }
    }

    void ExplorerTreeView::importServer()
    {
        QModelIndex sel = selectedIndex();
        if(!sel.isValid()){
            return;
        }

        ExplorerServerItem *node = common::utils_qt::item<ExplorerServerItem*>(sel);
        if(!node){
            return;
        }

        IServerSPtr server = node->server();

        using namespace translations;
        QString filepath = QFileDialog::getOpenFileName(this, trImport, QString(), trfilterForRdb);
        if (filepath.isEmpty() && server) {
            server->exportFromPath(filepath);
        }
    }

    void ExplorerTreeView::shutdownServer()
    {
        QModelIndex sel = selectedIndex();
        if(!sel.isValid()){
            return;
        }

        ExplorerServerItem *node = common::utils_qt::item<ExplorerServerItem*>(sel);
        if(!node){
            return;
        }

        IServerSPtr server = node->server();
        if(server && server->isConnected()){
            // Ask user
            int answer = QMessageBox::question(this, "Shutdown", QString("Really shutdown \"%1\" server?").arg(server->name()), QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

            if (answer != QMessageBox::Yes){
                return;
            }

            server->shutDown();
        }
    }

    void ExplorerTreeView::loadContentDb()
    {
        QModelIndex sel = selectedIndex();
        if(!sel.isValid()){
            return;
        }

        ExplorerDatabaseItem *node = common::utils_qt::item<ExplorerDatabaseItem*>(sel);
        if(node){
            LoadContentDbDialog loadDb(QString("Load %1 content").arg(node->name()), node->server()->type(), this);
            int result = loadDb.exec();
            if(result == QDialog::Accepted){
                node->loadContent(common::convertToString(loadDb.pattern()), loadDb.count());
            }
        }
    }

    void ExplorerTreeView::setDefaultDb()
    {
        QModelIndex sel = selectedIndex();
        if(!sel.isValid()){
            return;
        }

        ExplorerDatabaseItem *node = common::utils_qt::item<ExplorerDatabaseItem*>(sel);
        if(node){
            node->setDefault();
        }
    }

    void ExplorerTreeView::createKey()
    {
        QModelIndex sel = selectedIndex();
        if(!sel.isValid()){
            return;
        }

        ExplorerDatabaseItem *node = common::utils_qt::item<ExplorerDatabaseItem*>(sel);
        if(node){
            CreateDbKeyDialog loadDb(QString("Create key for %1 database").arg(node->name()), node->server()->type(), this);
            int result = loadDb.exec();
            if(result == QDialog::Accepted){
                FastoObjectIPtr val = loadDb.value();
                NKey key = loadDb.key();
                node->createKey(key, val);
            }
        }
    }

    void ExplorerTreeView::getValue()
    {
        QModelIndex sel = selectedIndex();
        if(!sel.isValid()){
            return;
        }

        ExplorerKeyItem *node = common::utils_qt::item<ExplorerKeyItem*>(sel);
        if(node){
            node->loadValueFromDb();
        }
    }

    void ExplorerTreeView::deleteKey()
    {
        QModelIndex sel = selectedIndex();
        if(!sel.isValid()){
            return;
        }

        ExplorerKeyItem *node = common::utils_qt::item<ExplorerKeyItem*>(sel);
        if(node){
            node->removeFromDb();
        }
    }

    void ExplorerTreeView::startLoadDatabases(const EventsInfo::LoadDatabasesInfoRequest& req)
    {

    }

    void ExplorerTreeView::finishLoadDatabases(const EventsInfo::LoadDatabasesInfoResponce& res)
    {
        common::ErrorValueSPtr er = res.errorInfo();
        if(er && er->isError()){
            return;
        }

        IServer *serv = qobject_cast<IServer *>(sender());
        DCHECK(serv);
        if(!serv){
            return;
        }

        ExplorerTreeModel *mod = qobject_cast<ExplorerTreeModel*>(model());
        DCHECK(mod);
        if(!mod){
            return;
        }

        EventsInfo::LoadDatabasesInfoResponce::database_info_cont_type dbs = res.databases_;

        for(int i = 0; i < dbs.size(); ++i){
            DataBaseInfoSPtr db = dbs[i];
            mod->addDatabase(serv, db);
        }
    }

    void ExplorerTreeView::startSetDefaultDatabase(const EventsInfo::SetDefaultDatabaseRequest& req)
    {

    }

    void ExplorerTreeView::finishSetDefaultDatabase(const EventsInfo::SetDefaultDatabaseResponce& res)
    {
        common::ErrorValueSPtr er = res.errorInfo();
        if(er && er->isError()){
            return;
        }

        IServer *serv = qobject_cast<IServer *>(sender());
        DCHECK(serv);
        if(!serv){
            return;
        }

        DataBaseInfoSPtr db = res.inf_;
        ExplorerTreeModel *mod = qobject_cast<ExplorerTreeModel*>(model());
        DCHECK(mod);
        if(!mod){
            return;
        }

        mod->setDefaultDb(serv, db);
    }

    void ExplorerTreeView::startLoadDatabaseContent(const EventsInfo::LoadDatabaseContentRequest& req)
    {

    }

    void ExplorerTreeView::finishLoadDatabaseContent(const EventsInfo::LoadDatabaseContentResponce& res)
    {
        common::ErrorValueSPtr er = res.errorInfo();
        if(er && er->isError()){
            return;
        }

        IServer *serv = qobject_cast<IServer *>(sender());
        DCHECK(serv);
        if(!serv){
            return;
        }

        ExplorerTreeModel *mod = qobject_cast<ExplorerTreeModel*>(model());
        DCHECK(mod);
        if(!mod){
            return;
        }

        EventsInfo::LoadDatabaseContentResponce::keys_cont_type keys = res.keys_;

        for(int i = 0; i < keys.size(); ++i){
            NKey key = keys[i];
            mod->addKey(serv, res.inf_, key);
        }
    }

    void ExplorerTreeView::startExecuteCommand(const EventsInfo::CommandRequest& req)
    {

    }

    void ExplorerTreeView::finishExecuteCommand(const EventsInfo::CommandResponce& res)
    {
        common::ErrorValueSPtr er = res.errorInfo();
        if(er && er->isError()){
            return;
        }

        IServer* serv = qobject_cast<IServer *>(sender());
        DCHECK(serv);
        if(!serv){
            return;
        }

        ExplorerTreeModel* mod = qobject_cast<ExplorerTreeModel*>(model());
        DCHECK(mod);
        if(!mod){
            return;
        }

        CommandKeySPtr key = res.cmd_;
        if(key->type() == CommandKey::C_DELETE){
            mod->removeKey(serv, res.inf_, key->key());
        }
        else if(key->type() == CommandKey::C_CREATE){
            mod->addKey(serv, res.inf_, key->key());
        }
    }

    void ExplorerTreeView::changeEvent(QEvent* e)
    {
        if(e->type() == QEvent::LanguageChange){
            retranslateUi();
        }

        QTreeView::changeEvent(e);
    }

    void ExplorerTreeView::retranslateUi()
    {
        using namespace translations;

        connectAction_->setText(tr("Connect/Disconnect"));
        openConsoleAction_->setText(trOpenConsole);
        loadDatabaseAction_->setText(trLoadDataBases);
        infoServerAction_->setText(trInfo);
        propertyServerAction_->setText(trProperty);
        historyServerAction_->setText(trHistory);
        closeAction_->setText(trClose);
        backupAction_->setText(trBackup);
        importAction_->setText(trImport);
        shutdownAction_->setText(trShutdown);

        loadContentAction_->setText(trLoadContOfDataBases);
        createKeyAction_->setText(trCreateKey);
        setDefaultDbAction_->setText(trSetDefault);
        getValueAction_->setText(trValue);
        deleteKeyAction_->setText(trDelete);
    }

    QModelIndex ExplorerTreeView::selectedIndex() const
    {
        QModelIndexList indexses = selectionModel()->selectedRows();

        if (indexses.count() != 1){
            return QModelIndex();
        }

        return indexses[0];
    }

    QModelIndexList ExplorerTreeView::selectedIndexes() const
    {
        return selectionModel()->selectedRows();
    }
}
