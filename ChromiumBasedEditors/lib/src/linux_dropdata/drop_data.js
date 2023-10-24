/*
 * (c) Copyright Ascensio System SIA 2010-2023
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at 20A-6 Ernesta Birznieka-Upish
 * street, Riga, Latvia, EU, LV-1050.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
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
	/* Есть проблема с пробрасыванием текста в input-элементы
	т.к. присвоение значения происходит системно, а не через eventListener
	логирование getData и getAsString показывает, что они не вызываются, поэтому, как решение - самим присваивать значение */
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
			/* Нет чёткого определения для получения текста: требуется text/plain(html), но применимо и просто text/Text, html/Html */
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