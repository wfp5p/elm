$set 3 #Elm
$ Yes answer letter - must be a single letter, lower case
$ #Yes
1	y
$ No answer letter - must be a single letter, lower case
$ #No
2	n
$quote "
$ #UnknownAddress
6	"'%s' is an unknown address.  Replace with: "
$ #CantBuildRetAddr
7	Can't build return address - hit MAX_HOPS limit!
$ #item
8	message
$ #items
9	messages
$ #Item
10	Message
$ #Items
11	Messages
$ #Prompt
12	"Command: "
$ #InitInternalTables
13	Initializing internal tables...
$quote
$ #ErrorExpanding
15	Error expanding "%s" - probable alias definition loop.
$ #IllegalAddress
16	%s is an illegal address!
$ #AliasExpTooLong
17	Alias expansion is too long.
$quote "
$ #MoreUser0
18	" There is 1 line left (%d%%). Press <space> for more, or 'i' to return. "
$ #MoreUser0Plural
19	" There are %d lines left (%d%%). Press <space> for more, or 'i' to return. "
$ #MoreUser1
20	" 1 line more (%d%%). Press <space> for more, 'i' to return. "
$ #MoreUser1Plural
21	" %d lines more (%d%%). Press <space> for more, 'i' to return. "
$ #MoreUser2
22	" 1 line more (you've seen %d%%) "
$ #MoreUser2Plural
23	" %d lines more (you've seen %d%%) "
$ #TitleDeleted
24	[deleted]
$ #TitleForm
25	Form
$ #TitleMessage
26	Message
$ #TitleTo
27	To
$ #TitleFrom
28	From
$ #TitlePage
29	"  Page %d"
$quote
$ #NoDelSysAlias
30	Can't delete a system alias!
$ #NoPathTo
31	Couldn't find a path to %s!
$ #PermFolder
32	Problems saving permissions of folder %s!
$ #InvokeEditor
33	Invoking editor...
$ #ProblemsInvokingEditor
34	Problems invoking editor %s!
$ #WarnNewMailRecv
35	Warning: new mail received...
$ #CouldntReopenTemp
36	Couldn't reopen temp file. Edit LOST!
$ #CouldntOpenFolder
37	Couldn't open %s for reading!  Edit LOST!
$ #CouldntSeekEnd
38	Couldn't seek to end of folder.  Edit LOST!
$ #CouldntCopyMailfile
39	Could not copy from "%s" to "%s"! [%s]
$ #ProblemsRestoringPerms
42	Problems restoring permissions of folder %s!
$ #ChangesIncorporated
43	Changes incorporated into new mail...
$ #ResyncingNewVersion
44	Resynchronizing with new version of folder...
$ #CantInvokeEditor
45	Can't invoke editor '%s' for composition.
$ #SimpleContinue
46	(Continue.)\n\r
$ #PostEdContinue
47	(Continue entering message.  Type ^D or '.' on a line by itself to end.)\n\r
$ #CouldntOpenAppend
48	Couldn't open %s for appending [%s].
$ #ContinueEntering
49	\n\rContinue entering message.
$ #EnterMessage
50	\n\rEnter message.
$ #TypeElmCommands
51	"  Type Elm commands on lines by themselves.\n\r"
$ #CommandsInclude
52	Commands include:  ^D or '.' to end, %cp to list, %c? for help.\n\n\r
$ #CantFindEmacs
53	\n\r(Can't find Emacs on this system! Continue.)\n\r
$ #DontKnowEmacs
54	\n\r(Don't know where Emacs would be. Continue.)\n\r
$ #EnterNameEditor
55	"\n\rPlease enter the name of the editor: "
$ #UseSpecificCommand
56	(You need to use a specific command here. Continue.)\n\r
$ #DontKnowChar
57	\n\r(Don't know what %c%c is. Try %c? for help.)\n\r
$ #EndOfMessage
58	\n\r<end-of-message>\n\n\n\n\r
$ #EditmsgAvailOpts
59	\n\r(Available options at this point are:\n\n\r
$ #EditmsgHelpMenu
60	\t%c?\tPrint this help menu.\n\r
$ #EditmsgAddLine
61	\t~~\tAdd line prefixed by a single '~' character.\n\r
$ #EditmsgBCC
62	\t%cb\tChange the addresses in the Blind-carbon-copy list.\n\r
$ #EditmsgCC
63	\t%cc\tChange the addresses in the Carbon-copy list.\n\r
$ #EditmsgEmacs
64	\t%ce\tInvoke the Emacs editor on the message, if possible.\n\r
$ #EditmsgAddMessage
65	\t%cf\tAdd the specified message or current.\n\r
$ #EditmsgToCCBCC
66	\t%ch\tChange all available headers (to, cc, bcc, subject).\n\r
$ #EditmsgSameCurrentPrefix
67	\t%cm\tSame as '%cf', but with the current 'prefix'.\n\r
$ #EditmsgUserEditor
68	\t%co\tInvoke a user specified editor on the message.\n\r
$ #EditmsgPrintMsg
69	\t%cp\tPrint out message as typed in so far.\n\r
$ #EditmsgReadFile
70	\t%cr\tRead in the specified file.\n\r
$ #EditmsgSubject
71	\t%cs\tChange the subject of the message.\n\r
$ #EditmsgTo
72	\t%ct\tChange the addresses in the To list.\n\r
$ #EditmsgVi
73	\t%cv\tInvoke the Vi visual editor on the message.\n\r
$ #EditmsgUnixCmd
74	\t%c!\tExecute a UNIX command (or give a shell if no command).\n\r
$ #EditmsgAddUnixCmd
75	\t%c<\tExecute a UNIX command adding the output to the message.\n\r
$ #EditmsgEndMsg
76	\t.  \tby itself on a line (or a control-D) ends the message.\n\r
$ #EditmsgContinue
77	Continue.)\n\r
$ #NoFilenameSpecified
78	\n\r(No filename specified for file read! Continue.)\n\r
$ #CouldntReadFile
79	\n\r(Couldn't read file '%s'! Continue.)\n\r
$ #AddedLine
80	\n\r(Added 1 line [
$ #AddedLinePlural
81	\n\r(Added %d lines [
$ #AddedChar
82	"1 char] "
$ #AddedCharPlural
83	"%d chars] "
$ #AddedFromFile
84	from file %s. Continue.)\n\r
$ #AddedToMessage
85	to message. Continue.)\n\r
$ #EditmsgPrintContinue
92	\n\r(Continue entering message.)\n\r
$ #NoMessageReadContinue
93	(No messages to read in! Continue.)\n\r
$ #ValidNumbersBetween
94	(Valid message numbers are between 1 and %d. Continue.)\n\r
$ #CantFindReadmsg
95	(Can't find 'readmsg' command! Continue.)\n\r
$ #EditmsgCouldntAdd
96	(Couldn't add the requested message. Continue.)\n\r
$ #EditmsgOneMoreCancel
97	(Interrupt. One more to cancel this letter.)\n\r
$ #EditmsgCancelled
98	(Interrupt. Letter canceled.)\n\r
$ #DecryptPrompt
99	"Enter decryption key: "
$ #FirstEncryptPrompt
100	"Enter encryption key: "
$ #SecondEncryptPrompt
101	"Please enter it again: "
$ #KeysNotSame
102	Your keys were not the same!
$ #AbandonChange
103	Abandon change to mailbox?
$ #AbandonChangePlural
104	Abandon changes to mailbox?
$quote
$ #SendOnlyMode
105	Send only mode [ELM %s]
$ #NewMailHangOn
106	New mail has arrived! Hang on...
$ #CantOpenFolderRead
107	Can't open '%s' for reading! [%s]
$ #NewMessageRecv
108	1 new message received.
$ #NewMessageRecvPlural
109	%d new messages received.
$ #ResyncFolder
110	Resynchronize folder
$ #NoMailToPipe
111	No mail to pipe!
$ #NoMailReturnAddress
112	No mail to get return address of!
$ #NoMailToScan
113	No mail to scan!
$ #PatternNotFound
114	pattern not found!
$ #ScanForCalendar
115	Scan message for calendar entries...
$ #SorryNoCalendar
116	Sorry. Calendar function disabled.
$ #BounceMessage
117	Bounce message
$ #NoMailToBounce
118	No mail to bounce!
$ #ChangeFolder
119	Change folder
$ #EditFolder
120	Edit folder
$ #FolderIsEmpty
121	Folder is empty!
$ #NoFolderEdit
122	Folder editing isn't configured in this version of ELM.
$ #Forward
123	Forward
$ #NoMailToForward
124	No mail to forward!
$ #GroupReply
125	Group reply
$ #CantGroupReplyForm
126	Can't group reply to a Form!!
$ #NoMailToReply
127	No mail to reply to!
$ #MessageWithHeaders
128	Message with headers...
$ #DisplayMessage
129	Display message
$ #NoMailToRead
130	No mail to read!
$ #LimitDisplayBy
131	Limit displayed %s by...
$ #Mail
132	Mail
$ #NextMessage
133	Next Message
$ #Options
134	Options
$ #PrintMail
135	Print mail
$ #NoMailToPrint
136	No mail to print!
$ #Quit
137	Quit
$ #NewMailQuitCancelled
138	New Mail!  Quit canceled...
$ #QuickQuit
139	Quick quit
$ #NewMailQuickQuitCancelled
140	New Mail!  Quick Quit canceled...
$ #ReplyToMessage
141	Reply to message
$ #NoMailToReplyTo
142	No mail to reply to!
$ #NoMailToSave
143	No mail to save!
$ #NoMailToCopy
144	No mail to copy!
$ #SaveToFolder
145	Save to folder
$ #CopyToFolder
146	Copy to folder
$ #QuickExit
147	Quick Exit
$ #Exit
148	Exit
$ #AlreadyOnLastPage
150	Already on last page.
$ #AlreadyOnFirstPage
151	Already on first page.
$ #NoItemToDelete
152	No %s to delete!
$ #NoMoreItemBelow
153	No more %s below.
$ #NoItemUndeletedBelow
154	No more undeleted %s below.
$ #NoMailInFolder
155	No mail in folder!
$ #NoAliases
156	No aliases!
$ #NoMoreItemAbove
157	No more %s above.
$ #NoMoreUndeletedAbove
158	No more undeleted %s above.
$ #NoItemToTag
159	No %s to tag!
$ #NoItemToMarkUndeleted
160	No %s to mark as undeleted!
$ #NoItemToUndelete
161	No %s to undelete!
$ #NewCurrentItem
162	New Current %s
$ #NotThatMany
163	Not that many %s.
$ #NotInLimitedDisplay
164	%s not in limited display.
$ #UnknownCommand
165	Unknown command. Use '?' for help.
$ #UseForHelp
166	(Use '?' for help)
$ #NoMailToCheck
167	No mail to check.
$ #Save
168	save
$ #Copy
169	copy
$ #CapSave
170	Save
$ #CapCopy
171	Copy
$ #Saved
172	saved
$ #Copied
173	copied
$ #SavecmdPrompt0
174	%s%s message
$ #SavecmdSaveMarked
175	Save marked messages?
$ #SavecmdPrompt1CurrentMessage
176	%s%s current message
$ #SavecmdPrompt1TaggedMessage
177	%s%s tagged message
$ #SavecmdPrompt1TaggedMessages
178	%s%s tagged messages
$ #SavecmdPrompt2
179	Select Folder for %s
$ #SavecmdAppendFile
180	Append to existing file "%s"?
$ #SavecmdAppendFolder
181	Append to mail folder "%s"?
$ #SavecmdCreateFolder
182	Create new mail folder "%s"?
$ #SavecmdCreateFile
183	Create new file "%s"?
$ #SavecmdMessageSavedTo
184	Message %d %s to "%s".
$ #SavecmdMessagesSaved
185	%d messages %s to "%s".
$ #SelfolderHelp
186	(Use "?" for help)
$ #SelfolderAlreadyReading
187	Already reading that folder!
$ #SelfolderCannotOpen
188	Cannot open "%s". [%s]
$ #DontKnowHomeCursor
203	Don't know what the home directory of "%s" is!
$ #DontKnowValueCursor
205	Don't know what the value of $%s is!
$ #ErrorFstat
207	Cannot fstat "%s"! [%s]
$ #CouldNotOpenFile
208	Could not open file %s.
$ #WriteFailedCopy
209	Write failed to temp file in copy(). [%s]
$ #CloseFailedCopy
210	Close failed to temp file in copy(). [%s]
$ #NoPermRead
213	You have no permission to read %s!\n\r
$ #NoMail
214	You have no mail.\n\r
$ #MailBeingForwarded
215	Your mail is being forwarded to %s.\n\r
$ #SeekFailed
216	ELM [seek] failed trying to read %d bytes into file.
$ #WriteCopyMessageFailed
217	Write failed in copy_message()! [%s]
$ #ErrorOpeningCheckFields
218	Error %s trying to open %s to check fields!
$ #CantReadMessageToValidate
219	Can't read the message to validate the form!
$ #CouldntOpenNewformOutput
220	Couldn't open newform file for form output!
$ #ErrorUnlinkingFile
221	Error %s unlinking file %s.
$ #ErrorLinkingFile
222	Error %s linking %s to %s.
$ #SeekFailedFile
223	ELM [seek] couldn't read %d bytes into file (%s).
$ #NoFormInMessage
224	No form in this message!?
$ #BadForm
225	Badly constructed form.  Can't reply!
$ #CantOpenAsOutputFile
226	Can't open "%s" as output file! (%s).
$ #EnterAsManyLines
227	(Enter as many lines as needed, ending with a '.' by itself on a line)\n
$ #FormNoPrompt
228	No Prompt Available:
$ #HdrmenuScreenTitle
229	Message Header Edit Screen
$ #HdrmenuInstruct
230	Choose header, u)ser defined header, d)omainize, !)shell, or <return>.
$ #HdrmenuInstructNoShell
231	Choose header, u)ser defined header, d)omainize, or <return>.
$quote "
$ #HdrmenuPrompt
232	"Choice: "
$ #HdrmenuBadChoice
233	No such header!
$ #HdrmenuGetExpiresInstruct
234	"In how many days should this message expire? "
$quote
$ #HdrmenuGetUserdefInstruct
235	Enter in the format "HeaderName: HeaderValue".
$ #HdrmenuGetInstruct
236	Enter value for the header.
$ #HdrmenuExpiresNotNumber
237	Expiration must be specified as a number of days.
$ #HdrmenuExpiresOutOfRange
238	Expiration date must be within eight weeks of today.
$ #HdrmenuPrecedenceBadValue
239	Unknown precedence value specified.
$ #HdrmenuUserdefNotAlnum
240	The user-defined header must begin with a letter or number.
$ #HdrmenuUserdefMissingColon
241	The user-defined header must have a colon after the field name.
$ #HdrmenuDomInstruct
242	Select header to domainize:  T)o, C)c, B)cc, or <return>.
$quote "
$ #HdrmenuDomPrompt
243	"Domainize choice: "
$quote
$ #HelpMessage
255	Press the key you want help for, '?' for a key list, or '.' to exit help
$ #HelpPrompt
$quote "
256	"Help for key: "
$ #HelpUnknownCommand
257	Unknown command.  Use '?' for a list of commands.
$ #HelpSystem
258	ELM Help System
$ #HelpCouldntOpenFile
259	Couldn't open file %s.
$ #HelpPressSpaceToContinue
260	Press <space> to continue, 'q' to return.
$ #HelpPressAnyKeyToReturn
261	Press any key to return.
$quote
$ #HelpDollar
262	$ = Force resynchronization of the current folder. This will purge deleted mail.
$ #HelpShell
263	! = Escape to the UNIX shell of your choice, or just to enter commands.
$ #HelpDebugSummary
264	@ = Debug - display a summary of the messages on the header page.
$ #HelpPipe
265	| = Pipe the current message or tagged messages to the command specified.
$ #HelpDebugAll
266	# = Debug - display all information known about current message.
$ #HelpDebugReturnAdd
267	% = Debug - display the computed return address of the current message.
$ #HelpLastMessage
268	* = Go to the last message in the current folder.
$ #HelpPreviousPage
269	- = Go to the previous page of messages.  This is the same as the LEFT arrow.
$ #HelpFirstMessage
270	'=' = Go to the first message in the current folder.
$ #HelpNextScreen
271	<space> = Display next screen of current message (or first screen of next).
$ #HelpDisplayCurrent
272	<space> = Display the current message.
$ #HelpNextPage
273	+ = Go to the next page of messages.  This is the same as the RIGHT arrow.
$ #HelpSearchFolder
274	/ = Search for specified pattern in folder.
$ #HelpCalendar
275	< = Scan current message for calendar entries (if enabled).
$ #HelpSave
276	> = Save current message or tagged messages to specified file.
$ #HelpToggleStatus
277	^ = Toggle the Delete/Undelete status of the current message.
$ #HelpAliasSubmenu
278	a = Enter the alias sub-menu section.  Create and display aliases.
$ #HelpBounce
279	b = Bounce (remail) a message to someone as if you have never seen it.
$ #HelpCopy
280	C = Copy current message or tagged messages to specified file.
$ #HelpChangeFolder
281	c = Change folders, leaving the current folder as if 'quitting'.
$ #HelpDelete
282	d = Mark the current message for future deletion.
$ #HelpDeletePattern
283	^D = Mark for deletion all messages with the specified pattern.
$ #HelpEditor
284	e = Invoke the editor on the entire folder, resynchronizing when done.
$ #HelpForward
285	f = Forward the current message to someone, return address is yours.
$ #HelpGroupReply
286	g = Group reply not only to the sender, but to everyone who received msg.
$ #HelpDisplayHeaders
287	h = Display message with all Headers (ignore weedout list).
$ #HelpReturnToIndex
288	i = Return to the index.
$ #HelpNextMessage
289	J = Go to the next message.
$ #HelpNextUndeleted
290	j = Go to the next undeleted message.  This is the same as the DOWN arrow.
$ #HelpPreviousMessage
291	K = Go to the previous message.
$ #HelpPreviousUndeleted
292	k = Go to the previous undeleted message.  This is the same as the UP arrow.
$ #HelpLimitDisplay
293	l = Limit displayed messages based on the specified criteria.
$ #HelpSendMail
294	m = Create and send mail to the specified person or persons.
$ #HelpDisplayNext
295	n = Display the next message.
$ #HelpDisplayThenNext
296	n = Display the current message, then move current to next message.
$ #HelpOptionsMenu
297	o = Go to the options submenu.
$ #HelpPrint
298	p = Print the current message or the tagged messages.
$ #HelpQuitPager
299	q = Quit the pager and return to the index.
$ #HelpQuitMailer
300	q = Quit the mailer, asking about deletion, saving, etc.
$ #HelpReplyMessage
301	r = Reply to the message.  This only sends to the originator of the message.
$ #HelpSaveMessage
302	s = Save current message or tagged messages to specified file.
$ #HelpTagMessage
303	t = Tag a message for further operations (or untag if tagged).
$ #HelpTagPattern
304	^T = Tag all messages with the specified pattern.
$ #HelpUndeleteMessage
305	u = Undelete - remove the deletion mark on the message.
$ #HelpExitFolder
306	x = Exit leaving the folder untouched, ask permission if changed.
$ #HelpQuickExit
307	X = Exit leaving the folder untouched, unconditionally.
$ #HelpQuickQuit
308	Q = Quick quit the mailer, save read, leave unread, delete deleted.
$ #HelpScrollForward
309	<return> = Display current message, or (builtin pager only) scroll forward.
$ #HelpDisplayCurrentMessage
310	<return> = Display the current message.
$ #HelpRewriteScreen
311	^L = Rewrite the screen.
$ #HelpExitQuickly
312	Exit the mail system quickly.
$ #HelpMakeMessageCurrent
313	<number> = Make specified number the current message.
$ #YouHaveNoPasswordEntry
314	You have no password entry!\n
$ #CouldNotOpenDebugFile
315	Could not open file %s for debug output!\n
$ #HelpMagic
316    & = Treat all folders as spool files.\n
$ #LeaveDeleteMessage
321	Delete message?
$ #LeaveDeleteMessages
322	Delete messages?
$ #LeaveMoveMessage
323	Move read message to "received" folder?
$ #LeaveMoveMessages
324	Move read messages to "received" folder?
$ #LeaveKeepMessage
325	Keep unread message in incoming mailbox?
$ #LeaveKeepMessages
326	Keep unread messages in incoming mailbox?
$ #SomethingWrongInCounts
327	Something wrong in message counts! Folder unchanged.
$ #FolderUnchanged
328	Folder unchanged.
$ #LeaveKeepStoreDelete
329	[Keeping 1 message, storing %d, and deleting %d.]
$ #LeaveKeepStoreDeletePlural
330	[Keeping %d messages, storing %d, and deleting %d.]
$ #LeaveKeepStore
331	[Keeping 1 message and storing %d.]
$ #LeaveKeepStorePlural
332	[Keeping %d messages and storing %d.]
$ #LeaveKeepDelete
333	[Keeping 1 message and deleting %d.]
$ #LeaveKeepDeletePlural
334	[Keeping %d messages and deleting %d.]
$ #LeaveKeep
335	[Keeping message.]
$ #LeaveKeepPlural
336	[Keeping all messages.]
$ #LeaveStoreDelete
337	[Storing 1 message and deleting %d.]
$ #LeaveStoreDeletePlural
338	[Storing %d messages and deleting %d.]
$ #LeaveStore
339	[Storing message.]
$ #LeaveStorePlural
340	[Storing all messages.]
$ #LeaveDelete
341	[Deleting all messages.]
$ #LeaveNewMailArrived
342	New mail has just arrived. Resynchronizing...
$ #LeaveAppendDenied
343	Permission to append to %s denied!  Leaving folder intact.
$ #LeaveCouldNotAppend
344	Could not append to folder %s!
$ #LeaveTempFileDenied
345	Permission to create temp file %s for writing denied! Leaving folder intact.
$ #LeaveCouldNotCreate
346	Could not create temp file %s!
$ #LeaveCloseFailedTemp
347	Close failed on temp file in leavembox()! [%s]
$ #LeaveProblemsSavingPerms
348	Problems saving permissions of folder %s!
$ #LeaveErrorOnStat
349	Error %s on stat(%s).
$ #LeaveLinkFailed
350	Link failed! [%s]
$ #LeaveCouldntModifyFolder
351	Couldn't modify folder!
$ #LeaveCantCopyMailbox
352	Can't copy folder!  Contents preserved in %s".
$ #LeaveSavedMailIn
353	Saved mail in %s.
$ #LeaveKeepingEmpty
354	Keeping empty folder '%s'.
$ #LeaveProblemsRestoringPerms
355	Problems restoring permissions of folder %s!
$ #LeaveChangingAccessTime
356	Error %s trying to change file %s access time.
$ #LeaveCouldntRemoveCurLock
357	Couldn't remove current lock file %s! [%s]
$ #LeaveErrorCreatingLock
358	Couldn't create lock file %s! [%s]
$ #LeaveWaitingToRead
360	Waiting to read mailbox while mail is being received: attempt #%d
$ #LeaveTimedOutRemoving
361	Timed out - removing current lock file...
$ #LeaveThrowingAwayLock
362	Throwing away the current lock file!
$ #LeaveGivingUp
364	Cannot lock folder - giving up.  Please try again in a few minutes.
$ #LeaveErrorTimedOutLock
366	Timed out on locking mailbox.  Leaving program.
$ #LeaveErrorReopenMailbox
367	Cannot re-open folder "%s"! [%s]
$ #LeaveErrorFlockMailbox
368	Cannot flock folder "%s"! [%s]
$ #LeaveCouldntUnlockOwnMailbox
369	Couldn't unlock my own mailbox %s!
$ #LeaveCouldntRemoveOwnLock
370	Couldn't remove my own lock file %s!
$quote "
$ #LimitEnterCriteria
371	"Enter criteria or '?' for help: "
$quote
$ #LimitAlreadyHave
372	Already have selection criteria - add more? (%c/%c) %c%c
$ #YesWord
373	Yes.
$ #LimitAdding
374	Adding criteria...
$ #NoWord
375	No.
$ #LimitChanging
376	Change criteria...
$ #LimitNotValidCriterion
377	"%s" not a valid criterion.
$ #LimitReturnToUnlimited
378	Returned to unlimited display.
$ #LimitMessagesSelected
379	%d messages selected.
$ #LimitMessageSelected
380	1 message selected.
$ #LimitNoMessagesSelected
381	No messages selected.
$quote "
$ #SetCurrentTo
382	"Set current %s to : "
$ #SendTheMessageTo
384	"Send the message to: "
$ #To
385	"To: "
$ #SubjectOfMessage
386	"Subject of message: "
$ #Subject
387	"Subject: "
$ #MailNotSent
388	"Mail not sent."
$ #NoSubjectContinue
389	No subject - Continue with message?
$ #MailNotSend
390	"Mail not sent."
$ #CopiesTo
391	"Copies to: "
$ #TooManyPeople
392	Too many people. Copies ignored.
$ #CopyMessageYN
393	Copy message?
$ #CopyMessageIntoReplyYN
394	Copy message into reply?
$quote
$ #InstalledSetuid
396	\n\
This version of Elm has been installed setuid=%d.  This is dangerous!\n\
Elm is NOT designed to run in this mode, and to do so can introduce\n\
grave security hazards.  See the description of "allow_setuid" in the\n\
Elm Reference Guide for further information.\n\n
$ #CouldNotMakeTemp
397	Sorry - couldn't make temp file name!
$ #SendingMail
399	Sending mail...
$ #MailerReturnedError
400	Mail failed!  [mailer exit status %d]
$ #MailSent
401	Mail sent!
$quote "
$ #RecallLastInstead
402	Recall last kept message instead?
$ #RecallLastKept
403	Recall last kept message?
$quote
$ #NoEncryptInBatch
411	Sorry.  Cannot send encrypted mail in "batch mode".
$ #WriteFailedSaveMssg
412	Write failed in save_mssg! [%s]
$ #BounceOffRemote
413	Bounce a copy off the remote machine?
$ #BackquoteCmdFailed
415	Backquoted command "%s" in elmheaders failed.
$ #AlreadyRunning1
416	You seem to have ELM already reading this mail!
$ #AlreadyRunning2
417	You may not have two copies of ELM running simultaneously.
$ #AlreadyRunning3
418	If this is in error, then you'll need to remove the following file:
$ #SorryCantUnlinkTemp
419	Sorry, can't unlink the temp file %s [%s]!
$ #FailOnOpenNewmbox
420	Cannot open "%s"! [%s]
$ #WhatsThisTempExists
421	What's this?  The temp folder "%s" already exists??
$ #CouldntOpenForTemp
423	Cannot open "%s"! [%s]
$ #CouldntReopenForTemp
424	Cannot reopen "%s"! [%s]
$ #ReadingInMessage
425	Reading in %s, message: %d
$ #ReadingInMessage0
426	Reading in %s, message: 0
$ #ReadingMessage0
427	Reading message: 0
$ #CouldntSeekEndFolder
428	Couldn't seek to %ld (end of folder) in %s! [%s]
$ #WriteToTempFailed
429	Write to "%s" failed! [%s]
$ #MailBeingForwardTo
430	Mail being forwarded to %s
$ #CloseOnFolderFailed
433	Close failed on folder "%s"! [%s]
$ #CloseOnTempFailed
434	Close failed on temp file "%s"! [%s]
$ #LengthNESpool
435	Huh!?  newmbox - length of mbox != spool mailbox length!!
$quote "
$ #On
436	"ON "
$quote
$ #Off
437	OFF
$ #SelectOptionLetter
438	Select letter of option line, '>' to save, or 'i' to return to index.
$ #SaveOptions
439	Save options in .elm/elmrc...
$ #CommandUnknown
440	Command unknown!
$ #OptionsEditor
441	-- ELM Options Editor --
$ #OptionCalendar
442	C)alendar file       : %s
$ #OptionDisplay
443	D)isplay mail using  : %s
$ #OptionEditor
444	E)ditor              : %s
$ #OptionFolder
445	F)older directory    : %s
$ #OptionSorting
446	S)orting criteria    : %s
$ #OptionAlias
447	A(l)ias Sorting      : %s
$ #OptionOutbound
448	O)utbound mail saved : %s
$ #OptionPrint
449	P)rint mail using    : %s
$ #OptionYourFullname
450	Y)our full name      : %s
$ #OptionArrow
451	A)rrow cursor        : %s
$ #OptionMenu
452	M)enu display        : %s
$ #OptionUserLevel
453	U)ser level          : %s
$ #OptionNamesOnly
454	N)ames only          : %s
$ #UseSpaceToToggle
455	(use <space> to toggle, any other key to leave)
$ #SpaceToChange
456	<space> to change
$ #SpaceForNext
457	(SPACE for next, or R)everse)
$ #ResortingFolder
458	Resorting folder...
$ #SortRSentDate
459	This sort will order most-recently-sent to least-recently-sent
$ #SortRRecvDate
460	This sort will order most-recently-received to least-recently-received
$ #SortRMailbox
461	This sort will order most-recently-added-to-folder to least-recently
$ #SortRSender
462	This sort will order by sender name, in reverse alphabetical order
$ #SortRSize
463	This sort will order messages by longest to shortest
$ #SortRSubject
464	This sort will order by subject, in reverse alphabetical order
$ #SortRStatus
465	This sort will order by reverse status - Deleted through Tagged...
$ #SortSentDate
466	This sort will order least-recently-sent to most-recently-sent
$ #SortRecvDate
467	This sort will order least-recently-received to most-recently-received
$ #SortMailbox
468	This sort will order least-recently-added-to-folder to most-recently
$ #SortSender
469	This sort will order by sender name
$ #SortSize
470	This sort will order messages by shortest to longest
$ #SortSubject
471	This sort will order messages by subject
$ #SortStatus
472	This sort will order by status - Tagged through Deleted...
$ #PressKeyHelp
486	Press the key you want help for, '?' for a key list, or '.' to exit help
$quote "
$ #KeyPrompt
487	"Key : "
$ #KeyIsntUsed
488	That key isn't used in this section.
$ #BeginningUser
489	"Beginning User   "
$ #IntermediateUser
490	"Intermediate User"
$ #ExpertUser
491	"Expert User      "
$quote
$ #ResortingAliases
492	Resorting aliases...
$ #ASortRAlias
493	This sort will order by alias name, in reverse alphabetical order
$ #ASortRName
494	This sort will order by user (real) name, in reverse alphabetical order
$ #ASortRText
495	This sort will order aliases in the reverse order as listed in aliases.text
$ #ASortAlias
496	This sort will order by alias name
$ #ASortName
497	This sort will order by user (real) name
$ #ASortText
498	This sort will order aliases in the order as listed in aliases.text
$ #FolderCorrupt
499	Folder is corrupt!!  I can't read it!!
$ #Tag
500	Tag
$ #Tagged
501	tagged
$ #CapTagged
502	Tagged
$ #Delete
503	Delete
$ #MarkDelete
504	marked for deletion
$ #CapMarkDelete
505	Marked for deletion
$ #Undelete
506	Undelete
$ #Undeleted
507	undeleted
$ #CapUndeleted
508	Undeleted
$quote "
$ #EnterPattern
509	"Enter pattern: "
$ #MessagesMatchPattern
510	%s %s that match pattern...
$ #SomeMessagesATagged
511	Some %s are already tagged.
$ #AMessageATagged
512	One %s is already tagged.
$ #RemoveTags
513	%s Remove Tags?
$ #RemoveTag
514	%s Remove Tag?
$ #TaggedMessages
515	%s %d %s.
$ #TaggedMessage
516	%s 1 %s.
$ #NoMatchesNoTags
517	No matches. No %s %s.
$ #MatchAnywhere
518	/ = Match anywhere in %s.
$ #MatchPattern
519	"Match pattern: "
$ #MatchPatternInEntire
520	"Match pattern (in entire %s): "
$ #SearchingFolderPattern
521	Searching folder for pattern...
$ #MatchSeekFailed
522	ELM [match] failed looking %ld bytes into file (%s).
$ #CouldntMallocBytes
523	Out of memory!  [%s couldn't allocate %d bytes]
$ #ChangeToWhichFolder
524	"Change to which folder: "
$quote
$ #ArgsHelp1
535	\nPossible Starting Arguments for ELM program:\n\n\r\
\targ\t\t\tMeaning\n\r\
\t -a \t\tArrow - use the arrow pointer regardless\n\r\
\t -c \t\tCheckalias - check the given aliases only\n\r\
\t -dn\t\tDebug - set debug level to 'n'\n\r\
\t -fx\t\tFolder - read folder 'x' rather than incoming mailbox\n\r\
\t -h \t\tHelp - give this list of options\n\r\
\t -ix\t\tInclude prepared file 'x' in edit buffer for send\n\r
$ #ArgsHelp2
536	\t -K \t\t- Enable use of HP 2622 softkeys\n\r\
\t -M \t\tMagic mode - treat all folders as spool files.\n\r
\t -m \t\tMenu - Turn off menu, using more of the screen\n\r\
\t -S \t\tSend-only mode\n\r\
\t -sx\t\tSubject 'x' - for batch mailing\n\r\
\t -t \t\tTiTe - don't use termcap/terminfo ti/te entries.\n\r\
\t -V \t\tEnable sendmail voyeur mode.\n\r\
\t -v \t\tPrint out ELM version information.\n\r\
\t -z \t\tZero - don't enter ELM if no mail is pending\n\r\
\n\n
$ #AlreadyReadingThatFolder
543	Already reading that folder!
$ #CantOpenFolderReadNONL
544	Can't open folder '%s' for reading!
$ #BatchDirNotice
545	\nNotice:\
\nThis version of ELM requires the use of a .elm directory to store your\
\nelmrc and alias files. I'd like to create the directory .elm for you\
\nand set it up, but I can't in "batch mode".\
\nPlease run ELM in "normal mode" first.\n
$ #DirNotice
546	\n\rNotice:\
\n\rThis version of ELM requires the use of a .elm directory in your home\
\n\rdirectory to store your elmrc and alias files. Shall I create the\
\n\rdirectory .elm for you and set it up (%c/%c/q)? %c%c
$ #DirNoticeNo
547	No.\n\rVery well. I won't create it.\n\rBut, you may run into difficulties later.\n\r
$ #DirNoticeYes
548	Yes.\n\rGreat! I'll do it now.\n\r
$ #DirNoticeQuit
549	Quit.\n\rOK.  Bailing out of ELM.\n\r
$ #ArgsIngoringDebug
550	Warning: system created without debugging enabled - request ignored\n
$ #ArgsTooManyAddresses
551	\n\rToo many addresses, or addresses too long!\n\r
$ #BadSortKeyInElmrc
555	I can't understand sort key "%s" in line %d in your ".elm/elmrc" file\n\r
$ #BadAliasSortInElmrc
556	I can't understand alias sort key "%s" in line %d in your ".elm/elmrc" file\n\r
$ #SignatureObsolete
557	"signature" used in obsolete way in .elm/elmrc file. Ignored!\n\r\
\t(Signature should specify the filename to use rather than on/off.)\n\r\n
$ #BouncebackGTMaxhops
558	Warning: bounceback is set to greater than %d (max-hops). Ignored.\n\r
$ #TimeoutLTTen
559	Warning: timeout is set to less than 10 seconds. Ignored.\n\r
$ #BadLineElmrc
560	I can't understand line %d in your ".elm/elmrc" file:\n\r> %s\n\r
$ #BatchNoticeFoldersDir
561	\nNotice:\n\
ELM requires the use of a folders directory to store your mail folders in.\n\
I'd like to create the directory %s for you,\n\
but I can't in "batch mode". Please run ELM in "normal mode" first.\n
$ #NoticeFoldersDir
562	\n\rNotice:\n\r\
ELM requires the use of a folders directory to store your mail folders in.\n\r\
Shall I create the directory %s for you (%c/%c/q)? %c%c
$ #TooManyWeedHeaders
563	Too many weed headers!  Leaving...\n\r
$ #CannotInitErrorExpanding
566	Cannot initialize "%s" - error expanding "%s".
$ #CouldntMakeTempFileName
567	Sorry - couldn't make file temp file name.
$ #CouldntOpenForWriting
568	Sorry - couldn't open file %s for writing (%s).
$ #SureYouWantToRemail
569	Are you sure you want to remail this message?
$ #BounceCancelled
570	Bounce of message canceled.
$ #ResendingMail
571	Resending mail...
$ #MailResent
572	Mail resent.
$ #FilledInForm
573	Filled in form
$ #ReYourMail
574	Re: your mail
$ #FormsCannotForward
575	Forms cannot be forwarded.  You might want to try "b)ounce" instead.
$ #ForwardedMail
576	Forwarded mail...
$ #BuildRAHitMaxHops
577	Can't build return address. Hit MAX_HOPS limit!
$ #NotAValidMessageNum
578	%d not a valid message number!
$ #CouldntSeekBytesIntoFlle
579	Couldn't seek %ld bytes into file (%s).
$ #EditOutgoingMessage
580	Edit outgoing message?
$ #CannotDetermineToName
581	Cannot determine `to' name to save by! Saving to "sent" folder %s instead.
$ #CannotSaveTo
582	Cannot save to %s!
$ #CannotSaveToNorSent
583	Cannot save to %s nor to "sent" folder %s!
$ #CannotSaveToSavingInstead
584	Cannot save to %s! Saving to "sent" folder %s instead.
$ #NoSave
586	<no save>
$ #UncondSaveByName
587	<unconditionally save by name>
$ #SentFolder
588	<"sent" folder>
$ #CondSaveByName
589	<conditionally save by name>
$quote "
$ #SaveCopyInPrompt
590	"Save copy in (use '?' for help/to list folders): "
$quote
$ #ListFoldersHelp
591	Enter: <nothing> to not save a copy of the message,\n\
\r       '<'       to save in your "sent" folder (%s),\n\
\r       '='       to save by name (the folder name depends on whom the\n\
\r                     message is to, in the end),\n\
\r       '=?'      to save by name if the folder already exists,\n\
\r                     and if not, to your "sent" folder,\n\
\r       or a filename (a leading '=' denotes your folder directory).\n\
\r\n\
\r
$ #ShownWithSelect
593	%s is '%s' with %d shown out of %d [ELM %s]
$ #ShownNoSelect
594	%s is '%s' with 1 message [ELM %s]
$ #FolderWord
595	Folder
$ #MailboxWord
596	Mailbox
$ #Level0MenuLine1
597	You can use any of the following commands by pressing the first character;
$ #Level0MenuLine2
598	d)elete or u)ndelete mail,  m)ail a message,  r)eply or f)orward mail,  q)uit
$ #Level0MenuLine3
599	To read a message, press <return>.  j = move down, k = move up, ? = help
$ #Level1MenuLine1
600	|=pipe, !=shell, ?=help, <n>=set current to n, /=search pattern
$ #Level1MenuLine2
601	a)lias, C)opy, c)hange folder, d)elete, e)dit, f)orward, g)roup reply, m)ail,
$ #Level1MenuLine3
602	n)ext, o)ptions, p)rint, q)uit, r)eply, s)ave, t)ag, u)ndelete, or e(x)it
$ #PreparePagerPipe
603	Could not prepare for external pager(pipe()-%s).
$ #PreparePagerFork
604	Could not prepare for external pager(fork()-%s).
$ #PreparePagerDup
605	Could not prepare for external pager(dup()-%s).
$ #PreparePagerChildFdopen
606	Could not prepare for external pager(child fdopen()-%s).
$ #PreparePagerParentFdopen
607	Could not prepare for external pager(parent fdopen()-%s).
$ #MessageAddressedTo
608	%s(message addressed to %.60s)\n
$ #TaggedPrivate
609	\n(** This message is tagged Private
$ #TaggedCompanyConfidential
610	\n(** This message is tagged Company Confidential
$ #TaggedUrgent
611	\n(** This message is tagged Urgent
$ #CommaUrgent
612	, Urgent
$quote "
$ #AndUrgent
613	" and Urgent"
$quote
$ #MessageHasExpired
614	\n(** This message has Expired
$ #AndHasExpired
615	, and has Expired
$ #PrematureEndOfFile
616	Premature end of file!
$quote "
$ #CommandIToReturn
617	" Command ('i' to return to index): "
$quote
$ #UseIToReturnIndex
618	(Use 'i' to return to index.)
$ #Pipe
619	Pipe
$ #SystemCall
620	System call
$ #ScanForCalendarCmd
621	Scan messages for calendar entries
$ #CantScanForCalendar
622	Can't scan for calendar entries!
$ #DisplayReturnAdd
623	Display return address
$ #MessageMarkedForDeleteion
625	Message marked for deletion.
$ #ForwardMessage
626	Forward message
$ #MailMessage
628	Mail message
$ #PrintMessage
629	Print message
$ #MessageTagged
631	Message tagged.
$ #MessageUntagged
632	Message untagged.
$ #MessageUndeleted
633	Message undeleted.
$ #Folder
634	Folder
$ #Mailbox
635	Mailbox
$quote "
$ #CommandLine
636	"%s Command ('i' to return to index): "
$quote
$ #StoppedUseFGToReturn
641	\n\nStopped.  Use "fg" to return to ELM\n\n
$ #BackInElmRedraw
642	\nBack in ELM. (You might need to explicitly request a redraw.)\n\n
$ #ShownNoSelectPlural
643	%s is '%s' with %d messages [ELM %s]
$quote '
$ #KeyF1
644	' Display   Msg'
$ #KeyF2
645	'  Mail     Msg'
$ #KeyF3
646	'  Reply  to Msg'
$ #Key0F4
647	'  Save     Msg'
$ #Key0F5
648	' Delete    Msg'
$ #Key0F6
649	' Undelete   Msg'
$ #Key1F4
650	' Change  Folder'
$ #Key1F5
651	'  Save     Msg'
$ #Key1F6
652	' Delete/Undelete'
$ #KeyF7
653	' Print     Msg'
$ #KeyF8
654	'  Quit     ELM'
$ #KeyAF1
655	' Alias  Current'
$ #KeyAF2
656	' Check  Person'
$ #KeyAF3
657	' Check  System'
$ #KeyAF4
658	' Make    Alias'
$ #KeyAF8
659	' Return  to ELM'
$ #KeyYF1
660	'  Yes'
$ #KeyYF8
661	'   No'
$ #KeyRF1
662	'  Next    Page  '
$ #KeyRF3
663	'  Next    Msg   '
$ #KeyRF4
664	'  Prev    Msg   '
$ #KeyRF5
665	'  Reply  to Msg '
$ #KeyRF6
666	' Delete   Msg   '
$ #KeyRF7
667	'  Send    Msg   '
$ #KeyRF8
668	' Return  to ELM '
$ #KeyCF1
669	'  Mail  Directry'
$ #KeyCF2
670	'  Home  Directry'
$ #KeyCF4
671	Incoming Mailbox
$ #KeyCF5
672	"Received" Folder
$ #KeyCF6
673	'"Sent"   Folder '
$ #KeyCF8
674	' Cancel'
$quote
$ #SortingMessagesBy
675	Sorting messages by %s...
$quote "
$ #AbrtRevDateMailSent
677	Reverse-Sent
$ #LongRevDateMailSent
678	Reverse Date Mail Sent
$ #AbrRevRecv
680	Reverse-Received
$ #LongRevRecv
681	Reverse Date Mail Rec'vd
$ #AbrRevMailbox
683	Reverse-Mailbox
$ #LongRevMailbox
684	Reverse Mailbox Order
$ #AbrRevSender
686	Reverse-From
$ #LongRevSender
687	Reverse Message Sender
$ #AbrRevLines
689	Reverse-Lines
$ #LongRevLines
690	Reverse Lines in Message
$ #AbrRevSubject
692	Reverse-Subject
$ #LongRevSubject
693	Reverse Message Subject
$ #AbrRevStatus
695	Reverse-Status
$ #LongRevStatus
696	Reverse Message Status
$ #AbrMailSent
698	Sent
$ #LongMailSent
699	Date Mail Sent
$ #AbrMailRecv
701	Received
$ #LongMailRecv
702	Date Mail Rec'vd
$ #AbrMailbox
704	Mailbox
$ #LongMailbox
705	Mailbox Order
$ #AbrSender
707	From
$ #LongSender
708	Message Sender
$ #AbrLines
710	Lines
$ #LongLines
711	Lines in Message
$ #AbrSubject
713	Subject
$ #LongSubject
714	Message Subject
$ #AbrStatus
716	Status
$ #LongStatus
717	Message Status
$ #GetTokenOverNested
718	Get_token calls nested greater than %d deep!
$ #UseShellName
719	(Use the shell name for a shell.)
$ #ShellCommand
720	"Shell command: "
$ #PressAnyKeyToReturn
721	"\n\nPress any key to return to ELM: "
$ #ReturnCodeWas
722	Return code was %d.
$ #PipeTo
723	"Pipe to: "
$quote
$ #TooManyMessagesSelected
730	Too many messages selected, messages from %d on not used
$ #ContentsOfYourFolderDir
731	\n\rContents of your folder directory:\n\r\n\r
$ #Change
738	change
$ #WindowSizeTooSmall
739	The window is too small to run Elm!
$ #SortUnknown
740	*UNKNOWN-SORT-PARAMETER*
$ #EnterLastSelected
741	Enter: {"subject","to","from"} [pattern] OR "all"
$ #EnterSelected
742	Enter: {"subject","to","from"} [pattern]
$ #HelpUndeletePattern
743	^U = Undelete all messages with the specified pattern.
$ #PressAnyKeyIndex
751	Press any key to return to index.
$ #ArgsInclFileBatch
753	\n\rCan't specify an included file in batch mode!\n\r
$ #OptionArrowMenu
760	A)rrow cursor
$ #OptionArrowHelp
761	This defines whether the ELM cursor is an arrow or a highlight bar.
$ #OptionBorderMenu
762	B)order on copy
$ #OptionBorderHelp
763	Left margin string (prefix) for reply and forward.
$ #OptionCalendarMenu
764	C)alendar file
$ #OptionCalendarHelp
765	This is the file where calendar entries from messages are saved.
$ #OptionDisplayMenu
766	D)isplay mail using
$ #OptionDisplayHelp
767	This is the program invoked to display individual messages (try 'builtin').
$ #OptionEditorMenu
768	E)ditor (primary)
$ #OptionEditorHelp
769	This is the editor used for all outbound messages (may be 'builtin').
$ #OptionFolderMenu
770	F)older directory
$ #OptionFolderHelp
771	This is the folders directory used when '=' (etc) is used in filenames.
$ #OptionHoldMenu
772	H)old sent message
$ #OptionHoldHelp
773	Whether to save a copy of outbound messages in O)utbound folder.
$ #OptionJreplyMenu
774	J) reply editor
$ #OptionJreplyHelp
775	This is the editor that will be used for replies (Editor="builtin").
$ #OptionKpauseMenu
776	K) pause after pager
$ #OptionKpauseHelp
777	Whether to pause (prompt) at end of message with display message pager.
$ #OptionaLiasMenu
778	A(l)ias Sorting
$ #OptionaLiasHelp
779	This is used to specify the sorting criteria for the alias display.
$ #OptionMenuMenu
780	M)enu display
$ #OptionMenuHelp
781	This determines if you have the mini-menu displayed or not.
$ #OptionNameMenu
782	N)ames only
$ #OptionNameHelp
783	Whether to display the names and addresses on mail, or names only.
$ #OptionOutboundMenu
784	O)utbound mail saved
$ #OptionOutboundHelp
785	This is where copies of outbound messages are saved automatically.
$ #OptionPrintMenu
786	P)rint mail using
$ #OptionPrintHelp
787	This is the command used to print mail messages.
$ #OptionReplyMenu
788	R)eply copies msg
$ #OptionReplyHelp
789	Whether to copy the message being replied to into the reply.
$ #OptionSortMenu
790	S)orting criteria
$ #OptionSortHelp
791	This is used to specify the sorting criteria for the folders
$ #OptionTextMenu
792	T)ext editor (~e)
$ #OptionTextHelp
793	This is the editor that will be used by the ~e command (Editor="builtin").
$ #OptionUserMenu
794	U)ser level
$ #OptionUserHelp
795	The level of knowledge you have about the ELM mail system.
$ #OptionVisualMenu
796	V)isual Editor (~v)
$ #OptionVisualHelp
797	This is the editor that will be used by the ~v command (Editor="builtin").
$ #OptionWantMenu
798	W)ant Cc: prompt
$ #OptionWantHelp
799	Whether to ask for "Copies to:" list when sending messages.
$ #OptionYourMenu
800	Y)our full name
$ #OptionYourHelp
801	When mail is sent out, this is what your full name will be recorded as.
$ #OptionZdashMenu
802	Z) signature dashes
$ #OptionZdashHelp
803	Whether to print dashes before including signature on outbound messages.
$ #CannotCreateFolderState
813	Cannot create folder state file "%s".
$ #CannotCreateEnvParam
814	Cannot create environment parameter "%s".
$ #CouldntSeekBytesIntoFolder
815	Couldn't seek %ld bytes into folder. [%s]
$ #PrintDontKnowHow
816	Don't know how to print - option "printmail" undefined!
$ #PrintPipeFailed
817	Cannot create pipe to print command.
$ #PrintPressAKey
818	Press any key to continue:
$ #PrintJobSpooled
819	Print job has been spooled.
$ #PrintFailCode
820	Printout failed with return code %d.
$ #PrintFailStatus
821	Printout failed with status 0x%04x.
$ #NoItemToScan
824	No %s to scan!
$ #ConfirmFilesAppend
825	Append to an existing file `%s'?
$ #ConfirmFolderAppend
826	Append to mail folder `%s'?
$ #ConfirmFilesCreate
827	Create a new file `%s'?
$ #ConfirmFolderCreate
828	Create a new mail folder `%s'?
$ #SavingToInstead
829	Alright - saving to `%s' instead
$ #HelpTagAndNext
830	T = Tag a message (or untag if tagged) and go to next message.
$ #CouldntSeekTempEnd
831	Couldn't fseek to end of reopened temp mbox! [%s]
$ #OptionCtrlLHelp
832	^L = Rewrite the screen.
$ #OptionGreaterHelp
833	> = Save to elmrc file.
$ #OptionReturnHelp
834	i,q = Return from option menu.
$ #ReadMessageIncrement
835	Warning: readmsginc is set to less than 1.  Ignored.\n\r
$ #SleepMessageInvalid
836	Warning: sleepmsg is set to less than 0.  Setting to 0.\n\r
$ #VfyPromptSendTheMsg
837	Send the message now? y
$ #VfyMenu1User0
838	Select letter of header to edit, 'e' to edit the message,
$ #VfyMenu2User0
839	'a' to make attachments, 'y' to send message, or 'n' to cancel.
$ #VfyMenuEditForm
840	e)dit form,
$ #VfyMenuEditMake
841	e)dit msg, m)ake form,
$ #VfyMenuEditMsg
842	e)dit message,
$quote "
$ #VfyMenuVfyCpy
843	"all h)eaders, a)ttachments, co(p)y, "
$ #VfyMenuIspell
844	"i)spell, "
$ #VfyMenuShell
845	"!)shell, "
$quote
$ #VfyMenuForget
846	f)orget, or:
$ #VfyMessageKept
847	Message kept.  Can be restored at next f)orward, m)ail or r)eply.
$ #VfyNoFieldsInForm
848	No fields in form!\007
$ #MagicOff
849	[Magic Off]
$ #MagicOn
850	[Magic On]
$ #MlistOff
851	[Mlists Off]
$ #MlistOn
852	[Mlists On]
$ #LeaveRenameFailed
853   Rename failed! [%s]
$ #TruncateFailedCopy
854   Error on ftruncate() of "%s"! [%s]
$ #NoStatusChange
855   Status changing isn't configured in this version of ELM.
$ #HelpStatusChange
856   S = Enter the message status change sub-menu.
$ #StatusEditor
857   -- ELM Status Editor --
$ #ToggleYes
858   YES
$ #ToggleNo
859   "NO "
$ #StatusCurrent
860   Current
$ #StatusOriginal
861   Original
$ #StatusNew
862   "New "
$ #StatusOld
863   "Old "
$ #StatusRead
864   Read
$ #StatusNewHelp
865   Whether the message is new and unread, old and unread, or already read.
$ #StatusRepliedHelp
866   Whether you've replied to the message.
$ #StatusOriginalHelp
867   Restore original status of message as of entry to elm or last resync.
$ #StatusUndoHelp
868   Undo changes made this time in the status editor.
$ #StatusReturnHelp
869   q,<return> = Return from status menu.
$ #StatusMenuLine1
870   Select letter of status line, 'o' to restore original status,
$ #StatusMenuLine2
871   'u' to undo changes, or <return> to return to index.
$ #VfyPromptSendMessage
872   Send message now? y
$ #VfySelectLetter
$quote "
873   "Select letter of header to edit, "
$quote
$ #VfyTitle
874   Send Message
$ #CopyMssgAttributionForwarded
875	Forwarded message:\n
$ #VfyFormToPlaintext
876	Converting form back to plaintext.  Do "m)ake form" to re-make form.
$ #CannotExpand
878	Cannot expand alias '%s'!
$ #FileOpenModeUpdate
880	update
$ #FileOpenModeRead
881	read
$ #FileOpenModeCreate
882	create
$ #FileOpenModeWrite
883	write
$ #FileOpenModeAppend
884	append
$ #FileOpenFailed
885	Cannot open "%s" for %s.  [%s]
$ #FileCloseFailed
886	Error closing "%s".  [%s]
$ #FileSeekErrorSeekingEnd
887	Error seeking %lu bytes from end in "%s".  [%s]
$ #FileSeekErrorSeekingAhead
888	Error seeking ahead %lu bytes in "%s".  [%s]
$ #FileSeekErrorSeekingPos
889	Error seeking byte %lu in "%s".  [%s]
$ #FileCopyErrorWritingFile
890	Error writing "%s".  [%s]
$ #FileCopyErrorWritingOutput
891	Error writing output file.  [%s]
$ #FileCopyErrorReadingFile
892	Error reading "%s".  [%s]
$ #FileCopyErrorReadingInput
893	Error reading input file.  [%s]
$ #FileRenameFailed
894	Error renaming "%s".  [%s]
$ #FileSeekEndFailed
895	Error seeking %lu bytes from end in "%s".  [%s]
$ #FileSeekCurFailed
896	Error seeking ahead %lu bytes in "%s".  [%s]
$ #FileSeekSetFailed
897	Error seeking byte %lu in "%s".  [%s]
$ #FileAccessFailed
898	Cannot access file "%s".  [%s]
$ #SigDescripInt
909	Interrupt
$ #SigDescripQuit
910	Quit
$ #SigDescripTerm
911	Terminate
$ #SigDescripHup
912	Hangup
$ #SigDescripUsr1
913	First User-Defined
$ #SigDescripUsr2
914	Second User-Defined
$ #SigDescripIll
915	Illegal Instruction
$ #SigDescripFpe
916	Floating Point Exception
$ #SigDescripBus
917	Bus Error
$ #SigDescripSegv
918	Segment Violation
$ #SigDescripAlrm
919	Alarm Clock
$ #SigDescripPipe
920	Write to Pipe
$ #SigDescripTstp
921	Stop from Terminal
$ #SigDescripCont
922	Continue Stopped Process
$ #SigDescripWinch
923	Window Size Change
$ #CopyMssgAttributionBegin
930	begin
$ #CopyMssgAttributionEnd
931	end
$ #CannotRefindInclusion
932	Cannot re-find inclusion located at offset %lu!
$ #InitScreenNoTerm
940	You must set the "TERM" environment parameter so that I know what\n\
kind of terminal you are using.\n
$ #InitScreenUnknownTerm
941	I cannot find any information on terminal type "%s" in your\n\
termcap/terminfo database.  Please check your "TERM" setting.\n
$ #InitScreenNoTcap
942	I cannot access your termcap/terminfo database to load the\n\
information on your terminal.\n
$ #InitScreenGetentFailed
943	Unable to retrieve termcap/terminfo database entry for your\n\
terminal (tgetent return code %d).\n
$ #InitScreenTcapcl
944	clear screen
$ #InitScreenTcapce
945	clear to end of line
$ #InitScreenTcapcd
946	clear to end of display
$ #InitScreenTcapcm
947	cursor motion
$ #InitScreenTcapcu
948	move cursor up
$ #InitScreenTcapnd
949	move cursor right
$ #InitScreenTcaple
950	move cursor left
$ #InitScreenGetattrFailed
951	Cannot retrieve attributes for your terminal line. [%s]\n"),
$ #InitScreenTcapMissing
952	Your terminal does not support the "%s" function (%s).\n
$ #FsyncFailedCopy
953	Error on fsync() of "%s"! [%s]
$ #EnterStringBadMode
954	INTERNAL ERROR - bad mode "%d" for enter_string
$ #LeaveEmergencyExitTaken
955	Emergency exit taken!
$ #LeavePreservingEditor
956	Preserving editor composition file "%s" ...
$ #LeavePreservingTemp
957	Preserving temporary folder "%s" ...
$ #LeaveDiscardingChanges
958	Discarding any changes made to mail folder ...
$ #LeavePreservingLock
959	Preserving folder lock file "%s" ...
$ #UseCheckaliasInstead
960	Error - the "-c" option is obsolete.\nUse the "checkalias" command instead\n
$ #ArgsNoFileBatch
961   \n\rMust specify a recipient in batch mode.\n\r
$ #NoRecipientsKeptMessage
962   No recipients specified!  Message kept.
$ #NoRecipients
963   No recipients specified!
