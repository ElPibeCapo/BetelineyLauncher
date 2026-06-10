#pragma once
#include <QObject>

#include "minecraft/auth/AuthStep.h"
#include "net/Download.h"
#include "net/NetJob.h"

class XboxProfileStep : public AuthStep {
    Q_OBJECT

   public:
    explicit XboxProfileStep(AccountData* data);
    virtual ~XboxProfileStep() noexcept = default;

    void perform() override;

    QString describe() override;

   private:
    void onRequestDone(QByteArray* response);

    Net::Download::Ptr m_request;
    NetJob::Ptr m_task;
};
