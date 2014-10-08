// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/protocol/input_filter.h"

#include "remoting/proto/event.pb.h"

namespace remoting {

// NormalizingInputFilterCros addresses the problems generated by key rewritings
// such as Down->PageDown, 1->F1, etc, when keys are pressed in combination with
// the OSKey (aka Search). Rewriting OSKey+Down, for example, causes us to
// receive the following:
//
// keydown OSKey
// keydown PageDown
// keyup PageDown
// keyup OSKey
//
// The host system will therefore behave as if OSKey+PageDown were pressed,
// rather than PageDown alone.
class NormalizingInputFilterCros : public protocol::InputFilter {
 public:
  explicit NormalizingInputFilterCros(protocol::InputStub* input_stub);
  virtual ~NormalizingInputFilterCros();

  // InputFilter overrides.
  virtual void InjectKeyEvent(const protocol::KeyEvent& event) override;
  virtual void InjectMouseEvent(const protocol::MouseEvent& event) override;

 private:
  void ProcessKeyDown(const protocol::KeyEvent& event);
  void ProcessKeyUp(const protocol::KeyEvent& event);

  void SwitchRewritingKeyToModifying();

  // Holds the keydown event for the most recent OSKey to have been pressed,
  // while it is Rewriting, or we are not yet sure whether it is Normal,
  // Rewriting or Modifying. The event is sent on if we switch to Modifying, or
  // discarded if the OSKey is released while in Rewriting mode.
  protocol::KeyEvent deferred_keydown_event_;

  // True while the |rewrite_keydown_event_| key is Rewriting, i.e. was followed
  // by one or more Rewritten key events, and not by any Modified events.
  bool deferred_key_is_rewriting_;

  // Stores the code of the OSKey while it is pressed for use as a Modifier.
  uint32 modifying_key_;

  DISALLOW_COPY_AND_ASSIGN(NormalizingInputFilterCros);
};

}  // namespace remoting
