/*
 *  Copyright (c) 2011 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_dynamic_sensor_list.h"
#include <QDomDocument>


KisDynamicSensorList::KisDynamicSensorList() : KisDynamicSensor(SensorsListId)
{
}

KisDynamicSensorList::~KisDynamicSensorList()
{
    qDeleteAll(m_list);
}

qreal KisDynamicSensorList::parameter(const KisPaintInformation& info)
{
    qreal t;
    foreach(KisDynamicSensor* sensor, m_list)
    {
        t *= sensor->parameter(info);
    }
    return t;
}

void KisDynamicSensorList::reset()
{
    foreach(KisDynamicSensor* sensor, m_list)
    {
        sensor->reset();
    }
}

void KisDynamicSensorList::toXML(QDomDocument& doc, QDomElement& elt) const
{
    foreach(KisDynamicSensor* sensor, m_list)
    {
        QDomElement childelt = doc.createElement("ChildSensor");
        sensor->toXML(doc, childelt);
        elt.appendChild(childelt);
    }    
}

void KisDynamicSensorList::fromXML(const QDomElement& elt)
{
    QDomNode node = elt.firstChild();
    while (!node.isNull())
    {
        if(node.isElement())
        {
            QDomElement childelt = node.toElement();
            if(childelt.tagName() == "ChildSensor")
            {
                KisDynamicSensor* sensor = KisDynamicSensor::createFromXML(childelt);
                m_list.push_back(sensor);
            }
        }
    }
}
