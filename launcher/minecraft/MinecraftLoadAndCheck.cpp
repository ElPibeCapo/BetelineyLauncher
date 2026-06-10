#include "MinecraftLoadAndCheck.h"
#include "MinecraftInstance.h"
#include "PackProfile.h"

MinecraftLoadAndCheck::MinecraftLoadAndCheck(MinecraftInstance* inst, Net::Mode netmode) : m_inst(inst), m_netmode(netmode) {}

void MinecraftLoadAndCheck::executeTask()
{
    auto components = m_inst->getPackProfile();
    auto result = components->reload(m_netmode);

    // Si Online falla (sin internet), intentar Offline como fallback
    if (!result && m_netmode == Net::Mode::Online) {
        result = components->reload(Net::Mode::Offline);
        if (result)
            qWarning() << "MinecraftLoadAndCheck: online falló, usando offline (libs ya descargadas)";
    }

    if (!result) {
        emitFailed(result.error);
        return;
    }
    m_task = components->getCurrentTask();

    if (!m_task) {
        emitSucceeded();
        return;
    }
    connect(m_task.get(), &Task::succeeded, this, &MinecraftLoadAndCheck::emitSucceeded);
    connect(m_task.get(), &Task::failed, this, &MinecraftLoadAndCheck::emitFailed);
    connect(m_task.get(), &Task::aborted, this, &MinecraftLoadAndCheck::emitAborted);
    propagateFromOther(m_task.get());
}

bool MinecraftLoadAndCheck::canAbort() const
{
    if (m_task) {
        return m_task->canAbort();
    }
    return true;
}

bool MinecraftLoadAndCheck::abort()
{
    if (m_task && m_task->canAbort()) {
        return m_task->abort();
    }
    return Task::abort();
}
