#ifndef SERVICECONTROLLER_H
#define SERVICECONTROLLER_H

#include <QString>

/** Controller class for starting and stopping system services. */
class ServiceController
{
public:
    /**
     * Start the service.
     *
     * @param errMsg (out) error message in the case of failure
     * @return true if successfully started, false otherwise
     */
    virtual bool start(QString& errMsg) = 0;

    /**
     * Stop the service.
     *
     * @param errMsg (out) error message in the case of failure
     * @return true if successfully stopped, false otherwise
     */
    virtual bool stop(QString& errMsg) = 0;

    /**
     * Inquire if the service is running.
     *
     * @param errMsg (out) error message in the case of failure
     * @return true if running, false otherwise
     */
    virtual bool running(QString& errMsg) = 0;

    /**
     * Get the human name of the service.
     *
     * @return human service name
     */
    QString humanName() const { return _humanName; }

    /**
     * Get the system name of the service.
     *
     * @return system service name
     */
    QString systemName() const { return _systemName; }

    /**
     * Create a service controller.
     *
     * @param humanName human service name
     * @param systemName system service name
     */
    static ServiceController* create(const QString& humanName, const QString& systemName);

protected:
    ServiceController(const QString& humanName, const QString& systemName) : _humanName(humanName), _systemName(systemName) {}

private:
    const QString _humanName;   /// name the service goes by to a human
    const QString _systemName;  /// name the service goes by to the system
};

#endif // SERVICECONTROLLER_H
