require "wxLua"
require "iox"


-- return the path part of the currently executing file
local strAt = string.byte('@')
function getPath()

    function findLast(filePath)
        local lastOffset = nil
        local offset
        repeat
            offset = string.find(filePath, "\\")
            if offset == nil then
                offset = string.find(filePath, "/")
            end
            if offset then
                if not lastOffset then
                    lastOffset = offset
                else
                    lastOffset = lastOffset + offset
                end
                filePath = string.sub(filePath, offset + 1)
            end
        until not offset
        return lastOffset
    end

    local filePath = debug.getinfo(1, "S").source
    if string.byte(filePath) == strAt then
		local offset = findLast(filePath)
        if offset ~= nil then
            -- remove the @ at the front up to just before the path separator
            filePath = string.sub(filePath, 2, offset - 1)
        else
            filePath = "."
        end
    else
        filePath = wx.wxGetCwd().."/Examples"
    end
    return filePath .. "\\"
end

ID_ADD=19
ID_REVERT=20

function DiffDialog()
	local DiffDialog= {}

    function GetSelection()
        return DiffDialog.fileList:GetNextItem(-1, wx.wxLIST_NEXT_ALL, wx.wxLIST_STATE_SELECTED);
    end

	function GetText(index)
		local listItem = wx.wxListItem()
		listItem:SetColumn(1)
		listItem:SetId(index)
		listItem:SetMask(wx.wxLIST_MASK_TEXT)
		DiffDialog.fileList:GetItem(listItem)
		return listItem:GetText()
	end

	function DiffFile()
		local fileName = GetText(GetSelection())
		if fileName then
			local diffFile = io.popen("svk diff \"" .. fileName .. "\"", 'r')
			diffFile:read('*a')
			diffFile:close()
		end
	end

	DiffDialog.frame=wx.wxDialog(wx.wxNull,-1,"", wx.wxPoint(0, 0), wx.wxSize(530, 455), wx.wxRESIZE_BORDER + wx.wxDEFAULT_DIALOG_STYLE)
	DiffDialog.frame:SetTitle('svk status')
	local itemmenu = 0
	DiffDialog.ItemRightClick=wx.wxMenu()
	itemmenu = wx.wxMenuItem(DiffDialog.ItemRightClick,ID_ADD,"Add...","",0)
	DiffDialog.ItemRightClick:AppendItem(itemmenu)
	itemmenu = wx.wxMenuItem(DiffDialog.ItemRightClick,ID_REVERT,"Revert...","",0)
	DiffDialog.ItemRightClick:AppendItem(itemmenu)
	DiffDialog.frame:ConnectEvent(ID_ADD, wx.wxEVT_COMMAND_MENU_SELECTED,
		function(event)
			local index = -1
			while true do
				index = DiffDialog.fileList:GetNextItem(index, wx.wxLIST_NEXT_ALL, wx.wxLIST_STATE_SELECTED);
				if index == -1 then
					break
				end

				local file = io.popen("svk add \"" .. GetText(index) .. "\"", 'r')
				file:read('*a')
				file:close()
			end
		end)
	DiffDialog.frame:ConnectEvent(ID_REVERT, wx.wxEVT_COMMAND_MENU_SELECTED,
		function(event)
			local index = -1
			while true do
				index = DiffDialog.fileList:GetNextItem(index, wx.wxLIST_NEXT_ALL, wx.wxLIST_STATE_SELECTED);
				if index == -1 then
					break
				end

				local file = io.popen("svk revert \"" .. GetText(index) .. "\"", 'r')
				file:read('*a')
				file:close()
			end
		end)

	DiffDialog.fileList = wx.wxListCtrl(DiffDialog.frame,-1,wx.wxPoint(6,6),wx.wxSize(512,411),wx.wxLC_REPORT)
	DiffDialog.fileList:ConnectEvent(wx.wxEVT_LEFT_DCLICK,
		function(event)
			DiffFile()
		end)
	DiffDialog.fileList:ConnectEvent(wx.wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,
		function(event)
			DiffDialog.theItemIndex = event:GetIndex()
			DiffDialog.frame:PopupMenu(DiffDialog.ItemRightClick, event:GetPoint())
		end)
	DiffDialog.sz6 = wx.wxBoxSizer(wx.wxVERTICAL)
	DiffDialog.sz9 = wx.wxBoxSizer(wx.wxVERTICAL)

	DiffDialog.sz6:AddSizer(DiffDialog.sz9,1,wx.wxTOP + wx.wxLEFT + wx.wxBOTTOM + wx.wxRIGHT + wx.wxEXPAND,3)
	DiffDialog.sz9:AddWindow(DiffDialog.fileList,1,wx.wxTOP + wx.wxLEFT + wx.wxBOTTOM + wx.wxRIGHT + wx.wxEXPAND,3)
	DiffDialog.frame:SetSizer(DiffDialog.sz6) DiffDialog.frame:SetAutoLayout(1)  DiffDialog.frame:Layout()
	DiffDialog.frame:Refresh()

	DiffDialog.fileList:InsertColumnInfo(0, "Status", wx.wxLIST_FORMAT_LEFT, 50)
	DiffDialog.fileList:InsertColumnInfo(1, "File name", wx.wxLIST_FORMAT_LEFT, DiffDialog.fileList:GetRect():GetWidth() - 50)
	return DiffDialog
end


function ScanPipe()
	local title
	local fullLine = ""
	for index = 1, table.getn(arg) do
		local str = arg[index]
		fullLine = fullLine .. str
	end
	local p = io.popen("svk st " .. fullLine, 'r')
	if not p then
		return
	end

	while true do
		if not title and frm then
			title = frm.frame:GetTitle()
			frm.frame:SetTitle(title .. ": Scanning...")
		end
		local str = p:read('*l')
		if not str then
			break
		end

		local _, _, status, fileName = string.find(str, "(.+)%s+(.+)")

		if status then
			local whichItem = frm.fileList:GetItemCount() + 1
			whichItem = frm.fileList:InsertStringItem(whichItem, status)
--			io.write(status .. "\t" .. str .. "\n")
			frm.fileList:SetStringItem(whichItem, 0, status)
			frm.fileList:SetStringItem(whichItem, 1, fileName)
		end
	end
	frm.frame:SetTitle(title)
end


function main()
    wx.wxInitAllImageHandlers()
    frm=DiffDialog()
	frm.frame:Centre()
--	frm.frame:ConnectEvent(-1, wx.wxEVT_IDLE, function(ev) OnIdle(ev) end)
end

--iox.SetCurrentDirectory("s:/LuaPlus/jjensen")

--thread.newthread(ScanPipe, {})
main()
ScanPipe()
frm.frame:ShowModal()

frm = nil
