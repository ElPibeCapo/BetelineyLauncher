// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Beteliney Launcher - Tests para la lógica de detección de GPU
 *
 *  Testea la función de clasificación de GPU que se usa tanto en
 *  lanzar.sh (bash) como en JavaSettingsWidget.cpp (C++/Qt).
 *
 *  La lógica se reimplementa aquí en C++ puro para poder testearla
 *  sin depender del sistema operativo ni de lspci.
 *
 *  Casos cubiertos:
 *    - iGPU AMD: Vega APU (Picasso/Renoir/Cezanne), Vega N genérico
 *    - iGPU Intel: UHD Graphics, HD Graphics, Iris, Iris Xe, Iris Pro
 *    - GPU discreta NVIDIA: GeForce, RTX, GTX, Quadro
 *    - GPU discreta AMD: RX series, Navi, Polaris
 *    - Vega 56/64/VII → discreta (NO iGPU)
 *    - Entrada vacía → unknown
 *    - String irreconocible → unknown
 *    - No confunde "Iris" con un apellido (string sin contexto GPU)
 *    - Múltiples GPUs: discreta + iGPU → "discrete" gana
 */

#include <QTest>
#include <QString>
#include <QStringList>

// ── Lógica de clasificación replicada desde lanzar.sh / JavaSettingsWidget ──
// Enum local para el test
enum class GpuType { IgpuAmd, IgpuIntel, Discrete, Unknown };

static GpuType classifyGpuList(const QString& lspciOutput)
{
    // Extraer líneas que sean VGA/3D/Display
    QStringList gpuLines;
    for (const QString& line : lspciOutput.split('\n')) {
        if (line.contains("VGA", Qt::CaseInsensitive) ||
            line.contains("3D controller", Qt::CaseInsensitive) ||
            line.contains("Display controller", Qt::CaseInsensitive)) {
            gpuLines << line;
        }
    }

    if (gpuLines.isEmpty())
        return GpuType::Unknown;

    // ¿Hay discreta?
    // NOTA: se verifica primero si la línea es iGPU AMD conocida — si lo es,
    // no se clasifica como discreta aunque contenga keywords de RX.
    // Ejemplo: "Phoenix [Radeon RX 7600M XT]" — Phoenix es iGPU, RX 7600M es mobile.
    static const QStringList igpuAmdPriorityKeywords = {
        "Phoenix", "Hawk Point", "Strix", "Raphael"
    };
    bool hasDiscrete = false;
    static const QStringList discreteKeywords = {
        "NVIDIA", "GeForce", "Quadro", "RTX", "GTX",
        "Navi", "Polaris",
        "RX 470", "RX 480", "RX 560", "RX 570", "RX 580",
        "RX 590", "RX 5", "RX 6", "RX 7",
        "Radeon RX Vega 56", "Radeon RX Vega 64", "Radeon VII"
    };
    for (const QString& line : gpuLines) {
        // Si la línea pertenece a una iGPU AMD de generación conocida, ignorarla
        // para la clasificación de discrete — aunque contenga "RX 7xxx".
        bool isKnownIgpu = false;
        for (const QString& kw : igpuAmdPriorityKeywords) {
            if (line.contains(kw, Qt::CaseInsensitive)) { isKnownIgpu = true; break; }
        }
        if (isKnownIgpu) continue;

        for (const QString& kw : discreteKeywords) {
            if (line.contains(kw, Qt::CaseInsensitive)) {
                hasDiscrete = true;
                break;
            }
        }
        if (hasDiscrete) break;
    }
    if (hasDiscrete)
        return GpuType::Discrete;

    // ¿iGPU AMD?
    static const QStringList igpuAmdKeywords = {
        "Vega 3", "Vega 6", "Vega 8", "Vega 10", "Vega 11",
        "Picasso", "Renoir", "Cezanne", "Rembrandt",
        "Phoenix", "Raphael", "Hawk Point", "Strix", "Kaveri", "Carrizo"
    };
    for (const QString& line : gpuLines) {
        for (const QString& kw : igpuAmdKeywords) {
            if (line.contains(kw, Qt::CaseInsensitive))
                return GpuType::IgpuAmd;
        }
    }

    // ¿iGPU Intel?
    static const QStringList igpuIntelKeywords = {
        "UHD Graphics", "HD Graphics", "Iris Xe", "Iris Pro", "Iris Plus", "Iris Graphics"
    };
    for (const QString& line : gpuLines) {
        for (const QString& kw : igpuIntelKeywords) {
            if (line.contains(kw, Qt::CaseInsensitive))
                return GpuType::IgpuIntel;
        }
    }

    return GpuType::Unknown;
}

class BetelineyGPUDetectTest : public QObject {
    Q_OBJECT

   private slots:

    // ── iGPU AMD ─────────────────────────────────────────────────────────────

    void test_vega10_is_igpu_amd()
    {
        QString input = R"(00:00.0 "VGA compatible controller" "AMD" "Radeon Vega 10 Graphics")";
        QCOMPARE(classifyGpuList(input), GpuType::IgpuAmd);
    }

    void test_picasso_is_igpu_amd()
    {
        QString input = R"(00:00.0 "VGA compatible controller" "AMD" "Picasso/Raven 2 [Radeon Vega Series]")";
        QCOMPARE(classifyGpuList(input), GpuType::IgpuAmd);
    }

    void test_renoir_is_igpu_amd()
    {
        QString input = R"(00:00.0 "VGA compatible controller" "AMD" "Renoir [Radeon RX Vega 6]")";
        QCOMPARE(classifyGpuList(input), GpuType::IgpuAmd);
    }

    void test_cezanne_is_igpu_amd()
    {
        QString input = R"(00:00.0 "VGA compatible controller" "AMD" "Cezanne [Radeon Vega Series]")";
        QCOMPARE(classifyGpuList(input), GpuType::IgpuAmd);
    }

    void test_rembrandt_is_igpu_amd()
    {
        QString input = R"(00:00.0 "VGA compatible controller" "AMD" "Rembrandt [Radeon 680M]")";
        QCOMPARE(classifyGpuList(input), GpuType::IgpuAmd);
    }

    void test_phoenix_is_igpu_amd()
    {
        QString input = R"(00:00.0 "VGA compatible controller" "AMD" "Phoenix [Radeon RX 7600M XT]")";
        QCOMPARE(classifyGpuList(input), GpuType::IgpuAmd);
    }

    void test_vega6_renoir_is_igpu_amd()
    {
        QString input = R"(00:00.0 "VGA compatible controller" "AMD" "Vega 6 [Radeon Vega Mobile GFX]")";
        QCOMPARE(classifyGpuList(input), GpuType::IgpuAmd);
    }

    // ── iGPU Intel ───────────────────────────────────────────────────────────

    void test_intel_uhd_is_igpu_intel()
    {
        QString input = R"(00:02.0 "VGA compatible controller" "Intel Corporation" "UHD Graphics 620")";
        QCOMPARE(classifyGpuList(input), GpuType::IgpuIntel);
    }

    void test_intel_hd_is_igpu_intel()
    {
        QString input = R"(00:02.0 "VGA compatible controller" "Intel Corporation" "HD Graphics 520")";
        QCOMPARE(classifyGpuList(input), GpuType::IgpuIntel);
    }

    void test_intel_iris_xe_is_igpu_intel()
    {
        QString input = R"(00:02.0 "VGA compatible controller" "Intel Corporation" "Iris Xe Graphics")";
        QCOMPARE(classifyGpuList(input), GpuType::IgpuIntel);
    }

    void test_intel_iris_pro_is_igpu_intel()
    {
        QString input = R"(00:02.0 "VGA compatible controller" "Intel Corporation" "Iris Pro Graphics 580")";
        QCOMPARE(classifyGpuList(input), GpuType::IgpuIntel);
    }

    // ── GPU discreta NVIDIA ───────────────────────────────────────────────────

    void test_nvidia_rtx_is_discrete()
    {
        QString input = R"(01:00.0 "VGA compatible controller" "NVIDIA" "GA106 [GeForce RTX 3060]")";
        QCOMPARE(classifyGpuList(input), GpuType::Discrete);
    }

    void test_nvidia_gtx_is_discrete()
    {
        QString input = R"(01:00.0 "VGA compatible controller" "NVIDIA" "TU117M [GeForce GTX 1650]")";
        QCOMPARE(classifyGpuList(input), GpuType::Discrete);
    }

    void test_nvidia_quadro_is_discrete()
    {
        QString input = R"(01:00.0 "3D controller" "NVIDIA" "Quadro T1000")";
        QCOMPARE(classifyGpuList(input), GpuType::Discrete);
    }

    // ── GPU discreta AMD ──────────────────────────────────────────────────────

    void test_amd_rx_navi_is_discrete()
    {
        QString input = R"(01:00.0 "VGA compatible controller" "AMD" "Navi 23 [Radeon RX 6600]")";
        QCOMPARE(classifyGpuList(input), GpuType::Discrete);
    }

    void test_amd_rx_polaris_is_discrete()
    {
        QString input = R"(01:00.0 "VGA compatible controller" "AMD" "Polaris 20 [Radeon RX 580]")";
        QCOMPARE(classifyGpuList(input), GpuType::Discrete);
    }

    // Vega 56/64/VII son DISCRETAS, no iGPU
    void test_vega56_is_discrete()
    {
        QString input = R"(01:00.0 "VGA compatible controller" "AMD" "Radeon RX Vega 56")";
        QCOMPARE(classifyGpuList(input), GpuType::Discrete);
    }

    void test_vega64_is_discrete()
    {
        QString input = R"(01:00.0 "VGA compatible controller" "AMD" "Radeon RX Vega 64")";
        QCOMPARE(classifyGpuList(input), GpuType::Discrete);
    }

    void test_radeon_vii_is_discrete()
    {
        QString input = R"(01:00.0 "VGA compatible controller" "AMD" "Radeon VII")";
        QCOMPARE(classifyGpuList(input), GpuType::Discrete);
    }

    // ── Dual GPU: discreta + iGPU → discrete gana ───────────────────────────

    void test_discrete_wins_over_igpu()
    {
        // Laptop con iGPU Intel + RTX discreta
        QString input =
            R"(00:02.0 "VGA compatible controller" "Intel Corporation" "UHD Graphics 630"
01:00.0 "VGA compatible controller" "NVIDIA" "GA107M [GeForce RTX 3050]")";
        QCOMPARE(classifyGpuList(input), GpuType::Discrete);
    }

    void test_amd_discrete_wins_over_vega_igpu()
    {
        QString input =
            R"(00:00.0 "VGA compatible controller" "AMD" "Picasso [Radeon Vega 8]"
01:00.0 "VGA compatible controller" "AMD" "Navi 22 [Radeon RX 6700 XT]")";
        QCOMPARE(classifyGpuList(input), GpuType::Discrete);
    }

    // ── Casos edge ────────────────────────────────────────────────────────────

    void test_empty_input_is_unknown()
    {
        QCOMPARE(classifyGpuList(""), GpuType::Unknown);
    }

    void test_no_gpu_lines_is_unknown()
    {
        // Líneas que no son VGA/3D/Display
        QString input = R"(00:00.0 "Host bridge" "AMD" "Something")";
        QCOMPARE(classifyGpuList(input), GpuType::Unknown);
    }

    void test_unrecognized_gpu_is_unknown()
    {
        // GPU real pero sin keywords conocidas (hipotética futura)
        QString input = R"(01:00.0 "VGA compatible controller" "SomeVendor" "FutureGPU 9000")";
        QCOMPARE(classifyGpuList(input), GpuType::Unknown);
    }

    void test_iris_not_misidentified_without_gpu_context()
    {
        // "Iris" sin contexto GPU no debe clasificar como iGPU Intel
        QString input = R"(00:00.0 "Host bridge" "SomeVendor" "Iris controller")";
        QCOMPARE(classifyGpuList(input), GpuType::Unknown);
    }

    void test_case_insensitive_match()
    {
        // keywords en minúsculas deben seguir matcheando
        QString input = R"(00:02.0 "VGA compatible controller" "intel corporation" "uhd graphics 770")";
        QCOMPARE(classifyGpuList(input), GpuType::IgpuIntel);
    }
};

QTEST_GUILESS_MAIN(BetelineyGPUDetectTest)
#include "BetelineyGPUDetect_test.moc"
