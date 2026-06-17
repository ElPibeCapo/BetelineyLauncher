// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Minecraft Launcher
 *  Copyright (C) 2026 El_PibeCapo <elpibecapoofficial@gmail.com>
 */
#pragma once

#include "launch/LaunchStep.h"
#include "launch/LaunchTask.h"

/**
 * Paso de lanzamiento que detecta mods duplicados o conflictivos
 * ANTES de que Minecraft inicie.
 *
 * Lee todos los .jar de la carpeta mods, extrae los mod_id usando
 * el mismo parser que usa Prism (ModUtils::process), busca IDs duplicados
 * y registra advertencias en el log visible del lanzamiento.
 *
 * No bloquea el lanzamiento — advierte y continúa.
 * Si el usuario quiere abortar puede hacerlo desde la UI.
 */
class CheckModConflicts : public LaunchStep {
    Q_OBJECT
public:
    explicit CheckModConflicts(LaunchTask* parent) : LaunchStep(parent) {}
    virtual ~CheckModConflicts() = default;

    void executeTask() override;
    bool canAbort() const override { return false; }
};
