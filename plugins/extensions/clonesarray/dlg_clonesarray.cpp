/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "dlg_clonesarray.h"

#include <klocalizedstring.h>

#include <KoColorSpaceConstants.h>

#include <kis_debug.h>
#include <KisViewManager.h>
#include <kis_image.h>
#include <kis_processing_applicator.h>
#include <commands/kis_image_commands.h>
#include <kis_node.h>
#include <kis_group_layer.h>
#include <kis_clone_layer.h>


DlgClonesArray::DlgClonesArray(KisViewManager *view, QWidget *parent)
    :   KoDialog(parent),
        m_view(view),
        m_applicator(0),
        m_baseLayer(m_view->activeLayer())
{
    Q_ASSERT(m_baseLayer);

    setCaption(i18n("Create Clones Array"));
    setButtons(Ok | Apply | Cancel);
    setDefaultButton(Ok);
    setObjectName("clones_array_dialog");

    m_page = new WdgClonesArray(this);
    Q_CHECK_PTR(m_page);
    m_page->setObjectName("clones_array");

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    connect(this, SIGNAL(okClicked()), SLOT(okClicked()));
    connect(this, SIGNAL(applyClicked()), SLOT(applyClicked()));
    connect(this, SIGNAL(cancelClicked()), SLOT(cancelClicked()));

    connect(m_page->columnXOffset, SIGNAL(valueChanged(int)), SLOT(syncOrthogonalToAngular()));
    connect(m_page->columnYOffset, SIGNAL(valueChanged(int)), SLOT(syncOrthogonalToAngular()));
    connect(m_page->rowXOffset, SIGNAL(valueChanged(int)), SLOT(syncOrthogonalToAngular()));
    connect(m_page->rowYOffset, SIGNAL(valueChanged(int)), SLOT(syncOrthogonalToAngular()));

    connect(m_page->columnDistance, SIGNAL(valueChanged(double)), SLOT(syncAngularToOrthogonal()));
    connect(m_page->columnAngle, SIGNAL(valueChanged(double)), SLOT(syncAngularToOrthogonal()));
    connect(m_page->rowDistance, SIGNAL(valueChanged(double)), SLOT(syncAngularToOrthogonal()));
    connect(m_page->rowAngle, SIGNAL(valueChanged(double)), SLOT(syncAngularToOrthogonal()));

    connect(m_page->numNegativeColumns, SIGNAL(valueChanged(int)), SLOT(setDirty()));
    connect(m_page->numPositiveColumns, SIGNAL(valueChanged(int)), SLOT(setDirty()));
    connect(m_page->numNegativeRows, SIGNAL(valueChanged(int)), SLOT(setDirty()));
    connect(m_page->numPositiveRows, SIGNAL(valueChanged(int)), SLOT(setDirty()));

    connect(m_page->numNegativeColumns, SIGNAL(valueChanged(int)), SLOT(updateCheckboxAvailability()));
    connect(m_page->numPositiveColumns, SIGNAL(valueChanged(int)), SLOT(updateCheckboxAvailability()));
    connect(m_page->numNegativeRows, SIGNAL(valueChanged(int)), SLOT(updateCheckboxAvailability()));
    connect(m_page->numPositiveRows, SIGNAL(valueChanged(int)), SLOT(updateCheckboxAvailability()));

    connect(m_page->columnPreference, SIGNAL(stateChanged(int)), SLOT(setDirty()));

    initializeValues();
    updateCheckboxAvailability();
}

DlgClonesArray::~DlgClonesArray()
{
    delete m_page;
}

void DlgClonesArray::initializeValues()
{
    if (m_baseLayer && m_baseLayer->original()) {
        QRect bounds = m_baseLayer->original()->exactBounds();
        m_page->columnXOffset->setValue(bounds.width());
        m_page->rowYOffset->setValue(bounds.height());
    }
}

void DlgClonesArray::setDirty()
{
    m_isDirty = true;
    enableButtonApply(m_isDirty);
}

void DlgClonesArray::setClean()
{
    m_isDirty = false;
    enableButtonApply(m_isDirty);
}

void DlgClonesArray::updateCheckboxAvailability()
{
    m_page->columnPreference->setEnabled(
        m_page->numNegativeColumns->value() > 0 ||
        m_page->numNegativeRows->value() > 0);
}

void DlgClonesArray::syncOrthogonalToAngular()
{
    setAngularSignalsEnabled(false);

    int x, y;

    x = m_page->columnXOffset->value();
    y = m_page->columnYOffset->value();
    m_page->columnDistance->setValue((float)sqrt(pow2(x) + pow2(y)));
    m_page->columnAngle->setValue(kisRadiansToDegrees(atan2((double) y, (double) x)));

    x = m_page->rowXOffset->value();
    y = m_page->rowYOffset->value();
    m_page->rowDistance->setValue((float)sqrt(pow2(x) + pow2(y)));
    m_page->rowAngle->setValue(kisRadiansToDegrees(atan2((double) y, (double) x)));

    setAngularSignalsEnabled(true);
    setDirty();
}

void DlgClonesArray::syncAngularToOrthogonal()
{
    setOrthogonalSignalsEnabled(false);

    qreal a, d;

    d = m_page->columnDistance->value();
    a = kisDegreesToRadians(m_page->columnAngle->value());
    m_page->columnXOffset->setValue(qRound(d * cos(a)));
    m_page->columnYOffset->setValue(qRound(d * sin(a)));

    d = m_page->rowDistance->value();
    a = kisDegreesToRadians(m_page->rowAngle->value());
    m_page->rowXOffset->setValue(qRound(d * cos(a)));
    m_page->rowYOffset->setValue(qRound(d * sin(a)));

    setOrthogonalSignalsEnabled(true);
    setDirty();
}

void DlgClonesArray::setOrthogonalSignalsEnabled(bool value)
{
    m_page->columnXOffset->blockSignals(!value);
    m_page->columnYOffset->blockSignals(!value);
    m_page->rowXOffset->blockSignals(!value);
    m_page->rowYOffset->blockSignals(!value);
}

void DlgClonesArray::setAngularSignalsEnabled(bool value)
{
    m_page->columnDistance->blockSignals(!value);
    m_page->columnAngle->blockSignals(!value);
    m_page->rowDistance->blockSignals(!value);
    m_page->rowAngle->blockSignals(!value);
}

void DlgClonesArray::okClicked()
{
    if (!m_applicator || m_isDirty) {
        reapplyClones();
    }

    Q_ASSERT(m_applicator);

    m_applicator->end();
    delete m_applicator;
    m_applicator = 0;
}

void DlgClonesArray::applyClicked()
{
    reapplyClones();
}

void DlgClonesArray::cancelClicked()
{
    if (m_applicator) {
        m_applicator->cancel();
        delete m_applicator;
        m_applicator = 0;
    }
}

void DlgClonesArray::reapplyClones()
{
    cancelClicked();

    KisImageSP image = m_view->image();

    if (!m_view->blockUntilOperationsFinished(image)) return;

    m_applicator =
        new KisProcessingApplicator(image, 0,
                                    KisProcessingApplicator::NONE,
                                    KisImageSignalVector() << ModifiedSignal);

    int columnXOffset = m_page->columnXOffset->value();
    int columnYOffset = m_page->columnYOffset->value();
    int rowXOffset = m_page->rowXOffset->value();
    int rowYOffset = m_page->rowYOffset->value();
    bool rowPreference = !m_page->columnPreference->isChecked();

    int startColumn = -m_page->numNegativeColumns->value();
    int startRow = -m_page->numNegativeRows->value();

    int endColumn = m_page->numPositiveColumns->value() - 1;
    int endRow = m_page->numPositiveRows->value() - 1;

    QString positiveGroupName = QString(i18n("+ Array of %1")).arg(m_baseLayer->name());
    KisGroupLayerSP positiveGroupLayer = new KisGroupLayer(image, positiveGroupName, OPACITY_OPAQUE_U8);
    m_applicator->applyCommand(new KisImageLayerAddCommand(image, positiveGroupLayer, m_baseLayer->parent(), m_baseLayer, false, true), KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);

    KisGroupLayerSP negativeGroupLayer;

    if (startRow < 0 || startColumn < 0) {
        QString negativeGroupName = QString(i18n("- Array of %1")).arg(m_baseLayer->name());
        negativeGroupLayer = new KisGroupLayer(image, negativeGroupName, OPACITY_OPAQUE_U8);
        m_applicator->applyCommand(new KisImageLayerAddCommand(image, negativeGroupLayer, m_baseLayer->parent(), m_baseLayer->prevSibling(), false, true), KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);
    }

    for (int row = endRow; row >= startRow; row--) {
        for (int col = endColumn; col >= startColumn; col--) {
            if (!col && !row) continue;

            bool choosePositiveGroup = rowPreference ? row > 0 || (row == 0 && col > 0) : col > 0 || (col == 0 && row > 0);
            KisNodeSP parent = choosePositiveGroup ? positiveGroupLayer : negativeGroupLayer;


            QString cloneName = QString("Clone %1, %2").arg(col).arg(row);
            KisCloneLayerSP clone = new KisCloneLayer(m_baseLayer, image, cloneName, OPACITY_OPAQUE_U8);
            clone->setX(-row * rowXOffset + col * columnXOffset);
            clone->setY(-row * rowYOffset + col * columnYOffset);

            m_applicator->applyCommand(new KisImageLayerAddCommand(image, clone, parent, 0, true, false), KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);
        }
    }

    setClean();
}

