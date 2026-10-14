#!/usr/bin/env python3
"""Generate synthetic XFA documents for the embedded viewer tests."""
from pathlib import Path
from xml.sax.saxutils import escape

from pypdf import PdfWriter
from pypdf.generic import (ArrayObject, BooleanObject, DecodedStreamObject,
    DictionaryObject, NameObject, TextStringObject)

HERE = Path(__file__).resolve().parent
FIXTURES = HERE / 'fixtures'
FIXTURES.mkdir(parents=True, exist_ok=True)
TNS = 'http://www.xfa.org/schema/xfa-template/3.3/'
DNS = 'http://www.xfa.org/schema/xfa-data/1.0/'

def write_fixture(filename, template, values):
    writer = PdfWriter()
    page = writer.add_blank_page(width=612, height=792)
    fallback = DecodedStreamObject()
    fallback.set_data(b'BT /F1 14 Tf 36 740 Td (Synthetic XFA fixture - an XFA viewer is required.) Tj ET')
    page[NameObject('/Contents')] = writer._add_object(fallback)
    page[NameObject('/Resources')] = DictionaryObject({NameObject('/Font'):DictionaryObject({
        NameObject('/F1'):writer._add_object(DictionaryObject({NameObject('/Type'):NameObject('/Font'),
            NameObject('/Subtype'):NameObject('/Type1'), NameObject('/BaseFont'):NameObject('/Helvetica')}))})})
    packets = [
        ('xdp:xdp','<?xml version="1.0" encoding="UTF-8"?><xdp:xdp xmlns:xdp="http://ns.adobe.com/xdp/">'),
        ('config','<config xmlns="http://www.xfa.org/schema/xci/3.0/"><present><pdf><version>1.7</version><dynamicRender>required</dynamicRender></pdf></present></config>'),
        ('template', template),
        ('datasets', f'<xfa:datasets xmlns:xfa="{DNS}"><xfa:data>{values}</xfa:data></xfa:datasets>'),
        ('</xdp:xdp>','</xdp:xdp>'),
    ]
    xfa = ArrayObject()
    for name, contents in packets:
        packet = DecodedStreamObject()
        packet.set_data(contents.encode('utf-8'))
        xfa.extend([TextStringObject(name), writer._add_object(packet)])
    fields = ArrayObject()
    writer._root_object[NameObject('/AcroForm')] = writer._add_object(DictionaryObject({
        NameObject('/Fields'):fields,NameObject('/XFA'):xfa}))
    writer._root_object[NameObject('/NeedsRendering')] = BooleanObject(True)
    writer.add_metadata({'/Title':filename.removesuffix('.pdf').replace('-',' '),
        '/Subject':'Synthetic XFA viewer regression fixture',
        '/Creator':'ONLYOFFICE XFA viewer tests'})
    writer.write(str(FIXTURES / filename))

PAGESET = '''<pageSet><pageArea name="PageArea1" id="PageArea1">
  <contentArea x="36pt" y="36pt" w="540pt" h="720pt"/>
  <medium short="612pt" long="792pt"/>
</pageArea></pageSet>'''

def wrap(body):
    return f'<template xmlns="{TNS}"><subform name="form1" layout="tb" locale="en_US">{PAGESET}{body}</subform></template>'

def xml_value(text):
    return escape(text).replace('\r','&#13;')

fixed_values = {'unicodeName':'Zoë García — Müller Ω',
    'notes':'First line: café, Straße, Ελληνικά.\r\rSecond paragraph retains its blank line.\rFinal line.',
    'emptyText':''}
fixed = wrap('''<subform name="Page1" w="540pt" h="720pt"><bind match="none"/>
  <draw x="0pt" y="0pt" w="500pt" h="24pt"><font typeface="Arial" size="14pt" weight="bold"/><value><text>SYNTHETIC FIXED XFA FORM</text></value></draw>
  <field name="unicodeName" x="0pt" y="40pt" w="480pt" h="36pt">
    <caption placement="top" reserve="14pt"><value><text>Unicode name</text></value></caption>
    <ui><textEdit/></ui><font typeface="Arial" size="12pt"/>
  </field>
  <field name="notes" x="0pt" y="100pt" w="480pt" h="144pt">
    <caption placement="top" reserve="16pt"><value><text>Multiline notes</text></value></caption>
    <ui><textEdit multiLine="1"/></ui><font typeface="Arial" size="11pt"/>
    <border><edge thickness="0.5pt"/></border>
  </field>
  <field name="emptyText" x="0pt" y="270pt" w="480pt" h="36pt">
    <caption placement="top" reserve="14pt"><value><text>Empty field (must remain editable)</text></value></caption>
    <ui><textEdit/></ui><font typeface="Arial" size="12pt"/>
  </field>
</subform>''')
write_fixture('fixed-unicode-multiline.pdf', fixed,
    '<form1>'+''.join(f'<{k}>{xml_value(v)}</{k}>' for k,v in fixed_values.items())+'</form1>')

flowed = wrap('''<subform name="Content" layout="tb" w="540pt"><bind match="none"/>
  <draw w="510pt" h="30pt"><font typeface="Arial" size="14pt" weight="bold"/><value><text>SYNTHETIC FLOWED XFA CONTROLS</text></value></draw>
  <field name="consent" w="510pt" h="24pt">
    <caption placement="right" reserve="480pt"><value><text>Checked consent option</text></value></caption>
    <ui><checkButton size="12pt" shape="square"/></ui>
    <items><text>yes</text><text>no</text></items><font typeface="Arial" size="11pt"/>
  </field>
  <field name="destination" w="510pt" h="42pt">
    <caption placement="top" reserve="16pt"><value><text>Destination (stored code differs from label)</text></value></caption>
    <ui><choiceList open="userControl"/></ui><font typeface="Arial" size="11pt"/>
    <items save="1"><text>DE</text><text>FR</text><text>JP</text></items>
    <items><text>Germany</text><text>France</text><text>Japan</text></items>
  </field>
  <exclGroup name="priority" layout="lr-tb" w="510pt" h="36pt">
    <field name="standard" w="220pt" h="24pt">
      <caption placement="right" reserve="194pt"><value><text>Standard priority</text></value></caption>
      <ui><checkButton size="12pt" shape="round"/></ui>
      <items><text>standard</text><text>off</text></items><font typeface="Arial" size="11pt"/>
    </field>
    <field name="expedited" w="220pt" h="24pt">
      <caption placement="right" reserve="194pt"><value><text>Expedited priority</text></value></caption>
      <ui><checkButton size="12pt" shape="round"/></ui>
      <items><text>expedited</text><text>off</text></items><font typeface="Arial" size="11pt"/>
    </field>
  </exclGroup>
  <subform name="entry" layout="tb" w="510pt">
    <occur min="0" max="-1" initial="1"/>
    <field name="description" w="510pt" h="58pt">
      <ui><textEdit multiLine="1"/></ui><font typeface="Arial" size="11pt"/>
      <border><edge thickness="0.5pt"/></border>
    </field>
  </subform>
</subform>''')
rows=[f'Row {i:02d} — repeated data, flowing across page boundaries.\rSecond line for synthetic row {i:02d}.' for i in range(1,13)]
flow_values='<form1><consent>yes</consent><destination>JP</destination><priority>expedited</priority>'
flow_values+=''.join(f'<entry><description>{xml_value(row)}</description></entry>' for row in rows)
flow_values+='</form1>'
write_fixture('flowed-controls-repeat.pdf', flowed, flow_values)
