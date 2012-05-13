#ifndef INTERFACE_H
#define INTERFACE_H

#include <QString>

class UpdMgrInterface
 {
 public:
     virtual ~UpdMgrInterface() {}
     virtual void checkForUpdate(const QString& appname, const QString& version) = 0;
     virtual QObject* updmgrObject() = 0;

 };

 Q_DECLARE_INTERFACE(UpdMgrInterface, "com.vgsoftware.Plugin.UpdMgrInterface/1.0");



#endif // INTERFACE_H
