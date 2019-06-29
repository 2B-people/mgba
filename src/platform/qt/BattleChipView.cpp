/* Copyright (c) 2013-2019 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "BattleChipView.h"

#include "CoreController.h"

#include <QFile>
#include <QStringList>

using namespace QGBA;

BattleChipView::BattleChipView(std::shared_ptr<CoreController> controller, QWidget* parent)
	: QDialog(parent)
	, m_controller(controller)
{
	m_ui.setupUi(this);

	char title[9];
	CoreController::Interrupter interrupter(m_controller);
	mCore* core = m_controller->thread()->core;
	title[8] = '\0';
	core->getGameCode(core, title);
	QString qtitle(title);


	connect(m_ui.chipId, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.inserted, [this]() {
		m_ui.inserted->setChecked(Qt::Unchecked);
	});
	connect(m_ui.chipName, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), m_ui.chipId, [this](int id) {
		m_ui.chipId->setValue(m_chipIndexToId[id]);
	});

	connect(m_ui.inserted, &QAbstractButton::toggled, this, &BattleChipView::insertChip);
	connect(controller.get(), &CoreController::stopping, this, &QWidget::close);

	connect(m_ui.gateBattleChip, &QAbstractButton::toggled, this, [this](bool on) {
		if (on) {
			setFlavor(GBA_FLAVOR_BATTLECHIP_GATE);
		}
	});
	connect(m_ui.gateProgress, &QAbstractButton::toggled, this, [this](bool on) {
		if (on) {
			setFlavor(GBA_FLAVOR_PROGRESS_GATE);
		}
	});
	connect(m_ui.gateBeastLink, &QAbstractButton::toggled, this, [this, qtitle](bool on) {
		if (on) {
			if (qtitle.endsWith('E') || qtitle.endsWith('P')) {
				setFlavor(GBA_FLAVOR_BEAST_LINK_GATE_US);
			} else {
				setFlavor(GBA_FLAVOR_BEAST_LINK_GATE);
			}
		}
	});

	m_controller->attachBattleChipGate();
	setFlavor(4);
	if (qtitle.startsWith("AGB-B4B") || qtitle.startsWith("AGB-B4W") || qtitle.startsWith("AGB-BR4") || qtitle.startsWith("AGB-BZ3")) {
		m_ui.gateBattleChip->setChecked(Qt::Checked);
	} else if (qtitle.startsWith("AGB-BRB") || qtitle.startsWith("AGB-BRK")) {
		m_ui.gateProgress->setChecked(Qt::Checked);
	} else if (qtitle.startsWith("AGB-BR5") || qtitle.startsWith("AGB-BR6")) {
		m_ui.gateBeastLink->setChecked(Qt::Checked);
	}
}

BattleChipView::~BattleChipView() {
	m_controller->detachBattleChipGate();
}

void BattleChipView::setFlavor(int flavor) {
	m_controller->setBattleChipFlavor(flavor);
	loadChipNames(flavor);
}

void BattleChipView::insertChip(bool inserted) {
	if (inserted) {
		m_controller->setBattleChipId(m_ui.chipId->value());
	} else {
		m_controller->setBattleChipId(0);
	}
}

void BattleChipView::loadChipNames(int flavor) {
	QStringList chipNames;
	chipNames.append(tr("(None)"));

	m_chipIndexToId.clear();
	if (flavor == GBA_FLAVOR_BEAST_LINK_GATE_US) {
		flavor = GBA_FLAVOR_BEAST_LINK_GATE;
	}

	QFile file(QString(":/res/chip-names-%1.txt").arg(flavor));
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	int id = 0;
	while (true) {
		QByteArray line = file.readLine();
		if (line.isEmpty()) {
			break;
		}
		++id;
		if (line.trimmed().isEmpty()) {
			continue;
		}
		m_chipIndexToId[chipNames.length()] = id;
		chipNames.append(QString::fromUtf8(line).trimmed());
	}

	m_ui.chipName->clear();
	m_ui.chipName->addItems(chipNames);
}