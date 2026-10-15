/*
 * Copyright (C) Ascensio System SIA, 2009-2026
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation, together with the
 * additional terms provided in the LICENSE file.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For
 * details, see the GNU AGPL at: https://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA by email at info@onlyoffice.com
 * or by postal mail at 20A-6 Ernesta Birznieka-Upisha Street, Riga,
 * LV-1050, Latvia, European Union.
 *
 * The interactive user interfaces in modified versions of the Program
 * are required to display Appropriate Legal Notices in accordance with
 * Section 5 of the GNU AGPL version 3.
 *
 * No trademark rights are granted under this License.
 *
 * All non-code elements of the Product, including illustrations,
 * icon sets, and technical writing content, are licensed under the
 * Creative Commons Attribution-ShareAlike 4.0 International License:
 * https://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 * This license applies only to such non-code elements and does not
 * modify or replace the licensing terms applicable to the Program's
 * source code, which remains licensed under the GNU Affero General
 * Public License v3.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

function createDataTransferItem(kind, type, data) {
	let item = { kind: kind, type: type, data: data,
		getAsFile: function() {
			return this.data;
		},
		getAsString: function(callback) {
			if (callback)
				callback(this.data);
		}
	};
	return item;
}

function createCustomEvent(type, x, y, c_x, c_y) {
	/* There is a problem with passing text through to input elements
	because the value is assigned by the system, not through an eventListener
	logging getData and getAsString shows they are never called, so the workaround is to assign the value ourselves */
	let event = new Event(type, { bubbles: true, cancelable: true, composed: true });
	event.dataTransfer = { dropEffect: 'none', effectAllowed: 'all', files: [], items: [], types: [], data: {},
		setData: function(type, value) { 
			this.effectAllowed = 'copyMove';
			this.data[type] = value;
			this.types.push(type);
			let dtItem = createDataTransferItem('string', type, value);
			this.items.push(dtItem);
		},
		getData: function(type) {
			/* There is no strict convention for requesting text: text/plain(html) is expected, but plain text/Text, html/Html are used too */
			let _type = type.toLowerCase(); 
			if (_type === 'text')
				_type = 'text/plain';
			else if (_type === 'html')
				_type = 'text/html';
			return this.data[_type];
		}
	};
	event.x = event.pageX = event.clientX = x; event.y = event.pageY = event.clientY = y; event.screenX = c_x; event.screenY = c_y;
	return event;
}

function addFileToDataTransfer(event, dataBase64, fileName) {
	let byteCharacters = atob(dataBase64);
	let byteNumbers = new Array(byteCharacters.length);
	for (let i = 0; i < byteCharacters.length; i++)
		byteNumbers[i] = byteCharacters.charCodeAt(i);
	let byteArray = new Uint8Array(byteNumbers); 
	let oFile = new File([byteArray], fileName);
	event.dataTransfer.files.push(oFile);
	let dtItem = createDataTransferItem('file', '', oFile);
	event.dataTransfer.items.push(dtItem);
}