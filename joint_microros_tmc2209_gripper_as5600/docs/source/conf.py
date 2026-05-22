project = 'joint_microros_tmc2209_gripper_as5600'
copyright = '2026, naj'
author = 'naj'
release = '0.1.0'

extensions = ['rst2pdf.pdfbuilder']

pdf_documents = [('index', u'joint_microros_tmc2209_gripper_as5600',
                  u'joint_microros_tmc2209_gripper_as5600',
                  u'naj'),]

pdf_break_level = 0  # No page breaks between sections
templates_path = ['_templates']
exclude_patterns = []
language = 'en'

html_theme = 'alabaster'
html_static_path = ['_static']
