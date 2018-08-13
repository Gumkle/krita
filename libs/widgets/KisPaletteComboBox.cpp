/*
 *  Copyright (c) 2018 Michael Zhou <simeirxh@gmail.com>
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

// Qt
#include <QPainter>
#include <QPen>
#include <QVector>

// STL
#include <algorithm>

#include "kis_palette_view.h"
#include "KisPaletteComboBox.h"

KisPaletteComboBox::KisPaletteComboBox(QWidget *parent)
    : QComboBox(parent)
    , m_completer(new QCompleter(QComboBox::model(), this))
    , m_model(Q_NULLPTR)
{
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setFilterMode(Qt::MatchContains);
    setCompleter(m_completer.data());
    connect(this, SIGNAL(currentIndexChanged(int)), SLOT(slotIndexUpdated(int)));
}

KisPaletteComboBox::~KisPaletteComboBox()
{ }

void KisPaletteComboBox::setPaletteModel(const KisPaletteModel *paletteModel)
{
    if (!m_model.isNull()) {
        m_model->disconnect(this);
    }
    m_model = paletteModel;
    if (m_model.isNull()) { return; }
    slotPaletteChanged();
    connect(m_model, SIGNAL(sigPaletteChanged()),
            SLOT(slotPaletteChanged()));
    connect(m_model, SIGNAL(sigPaletteModified()),
            SLOT(slotPaletteChanged()));
}

void KisPaletteComboBox::setCompanionView(KisPaletteView *view)
{
    if (!m_view.isNull()) {
        m_view->disconnect(this);
        disconnect(m_view.data());
    }
    m_view = view;
    setPaletteModel(view->paletteModel());
    connect(view, SIGNAL(sigIndexSelected(QModelIndex)),
            SLOT(slotSwatchSelected(QModelIndex)));
    connect(this, SIGNAL(sigColorSelected(KoColor)),
            view, SLOT(slotFGColorChanged(KoColor)));
}

void KisPaletteComboBox::slotPaletteChanged()
{
    if (QPointer<KoColorSet>(m_model->colorSet()).isNull()) { return; }

    clear();
    m_groupMapMap.clear();
    m_idxSwatchMap.clear();

    for (const QString &groupName : m_model->colorSet()->getGroupNames()) {
        QVector<SwatchInfoType> infoList;
        PosIdxMapType posIdxMap;
        const KisSwatchGroup *group = m_model->colorSet()->getGroup(groupName);
        for (const SwatchInfoType &info : group->infoList()) {
            infoList.append(info);
        }
        std::sort(infoList.begin(), infoList.end(), swatchInfoLess);
        for (const SwatchInfoType &info : infoList) {
            const KisSwatch &swatch = info.swatch;
            QString name = swatch.name();
            if (!swatch.id().isEmpty()){
                name = swatch.id() + " - " + swatch.name();
            }
            addItem(QIcon(createColorSquare(swatch)), name);
            posIdxMap[SwatchPosType(info.column, info.row)] = count() - 1;
            m_idxSwatchMap.push_back(swatch);
        }
        m_groupMapMap[group->name()] = posIdxMap;
    }
    if (!m_view.isNull()) {
        setCurrentIndex(0);
    }
    QModelIndex idx = m_view->currentIndex();
    if (!idx.isValid()) { return; }
    if (!qvariant_cast<bool>(idx.data(KisPaletteModel::IsGroupNameRole))) { return; }
    if (!qvariant_cast<bool>(idx.data(KisPaletteModel::CheckSlotRole))) { return; }

    QString groupName = qvariant_cast<QString>(idx.data(KisPaletteModel::GroupNameRole));
    int rowInGroup = qvariant_cast<int>(idx.data(KisPaletteModel::RowInGroupRole));

    blockSignals(true); // this is a passive selection; this shouldn't make others change
    setCurrentIndex(m_groupMapMap[groupName][SwatchPosType(idx.column(), rowInGroup)]);
    blockSignals(false);
}

bool KisPaletteComboBox::swatchInfoLess(const SwatchInfoType &first, const SwatchInfoType &second)
{
    return first.swatch.name() < second.swatch.name();
}

QPixmap KisPaletteComboBox::createColorSquare(const KisSwatch &swatch) const
{
    QPixmap colorSquare(32, 32);
    if (swatch.spotColor()) {
        QImage img = QImage(32, 32, QImage::Format_ARGB32);
        QPainter circlePainter;
        img.fill(Qt::transparent);
        circlePainter.begin(&img);
        QBrush brush = QBrush(Qt::SolidPattern);
        brush.setColor(swatch.color().toQColor());
        circlePainter.setBrush(brush);
        QPen pen = circlePainter.pen();
        pen.setColor(Qt::transparent);
        pen.setWidth(0);
        circlePainter.setPen(pen);
        circlePainter.drawEllipse(0, 0, 32, 32);
        circlePainter.end();
        colorSquare = QPixmap::fromImage(img);
    } else {
        colorSquare.fill(swatch.color().toQColor());
    }
    return colorSquare;
}

void KisPaletteComboBox::slotSwatchSelected(const QModelIndex &index)
{
    QString gName = qvariant_cast<QString>(index.data(KisPaletteModel::GroupNameRole));
    int rowInGroup = qvariant_cast<int>(index.data(KisPaletteModel::RowInGroupRole));
    setCurrentIndex(m_groupMapMap[gName][SwatchPosType(index.column(), rowInGroup)]);
}

void KisPaletteComboBox::slotIndexUpdated(int idx)
{
    if (idx >= 0 && idx < m_idxSwatchMap.size()) {
        emit sigColorSelected(m_idxSwatchMap[idx].color());
    }
}
