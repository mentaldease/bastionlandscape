import "dotnet"

load_assembly("System.Windows.Forms")
load_assembly("System.Drawing")

Form = import_type("System.Windows.Forms.Form")
ListView = import_type("System.Windows.Forms.ListView")
ColumnHeader = import_type("System.Windows.Forms.ColumnHeader")
ColumnHeaderStyle = import_type("System.Windows.Forms.ColumnHeaderStyle")
Point = import_type("System.Drawing.Point")
Size = import_type("System.Drawing.Size")
View = import_type("System.Windows.Forms.View")

form = Form()
listView = ListView()
form:SuspendLayout()

columnHeader = ColumnHeader()
columnHeader.Text = "Message";
columnHeader.Width = 800;

listView.Columns:Add(columnHeader)
listView.Location = Point(8, 8)
listView.Name = "listView1";
listView.Size = Size(280, 248)
listView.TabIndex = 0
listView.View = View.Details
listView.FullRowSelect = true
listView.HeaderStyle = ColumnHeaderStyle.None
listView.MultiSelect = false

form.AutoScaleBaseSize = Size(5, 13)
form.ClientSize = Size(292, 266)
form.Controls:Add(listView)
form.Name = "Form1"
form.Text = "Form1"

form:ResumeLayout(false)
form:ShowDialog()

