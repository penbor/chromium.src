// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/host/single_window_input_injector.h"

#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>

#include "base/mac/foundation_util.h"
#include "base/mac/scoped_cftyperef.h"
#include "remoting/proto/event.pb.h"
#include "third_party/webrtc/modules/desktop_capture/mac/desktop_configuration.h"

namespace remoting {

using protocol::ClipboardEvent;
using protocol::KeyEvent;
using protocol::TextEvent;
using protocol::MouseEvent;

class SingleWindowInputInjectorMac : public SingleWindowInputInjector {
 public:
  SingleWindowInputInjectorMac(
      webrtc::WindowId window_id,
      scoped_ptr<InputInjector> input_injector);
  virtual ~SingleWindowInputInjectorMac();

  // InputInjector interface.
  virtual void Start(
      scoped_ptr<protocol::ClipboardStub> client_clipboard) override;
  virtual void InjectKeyEvent(const KeyEvent& event) override;
  virtual void InjectTextEvent(const TextEvent& event) override;
  virtual void InjectMouseEvent(const MouseEvent& event) override;
  virtual void InjectClipboardEvent(const ClipboardEvent& event) override;

 private:
  CGRect FindCGRectOfWindow();

  CGWindowID window_id_;
  scoped_ptr<InputInjector> input_injector_;

  DISALLOW_COPY_AND_ASSIGN(SingleWindowInputInjectorMac);
};

SingleWindowInputInjectorMac::SingleWindowInputInjectorMac(
    webrtc::WindowId window_id,
    scoped_ptr<InputInjector> input_injector)
    : window_id_(static_cast<CGWindowID>(window_id)),
      input_injector_(input_injector.Pass()) {
}

SingleWindowInputInjectorMac::~SingleWindowInputInjectorMac() {
}

void SingleWindowInputInjectorMac::Start(
    scoped_ptr<protocol::ClipboardStub> client_clipboard) {
  input_injector_->Start(client_clipboard.Pass());
}

void SingleWindowInputInjectorMac::InjectKeyEvent(const KeyEvent& event) {
  input_injector_->InjectKeyEvent(event);
}

void SingleWindowInputInjectorMac::InjectTextEvent(const TextEvent& event) {
  input_injector_->InjectTextEvent(event);
}

void SingleWindowInputInjectorMac::InjectMouseEvent(const MouseEvent& event) {
  if (event.has_x() && event.has_y()) {
    CGRect window_rect = FindCGRectOfWindow();
    if (CGRectIsNull(window_rect)) {
      LOG(ERROR) << "Window rect is null, so forwarding unmodified MouseEvent";
      input_injector_->InjectMouseEvent(event);
      return;
    }

    webrtc::MacDesktopConfiguration desktop_config =
        webrtc::MacDesktopConfiguration::GetCurrent(
            webrtc::MacDesktopConfiguration::TopLeftOrigin);

    // Create a vector that has the origin of the window.
    webrtc::DesktopVector window_pos(window_rect.origin.x,
                                     window_rect.origin.y);

    // The underlying InputInjector expects coordinates relative to the
    // top-left of the top-left-most monitor, so translate the window origin
    // to that coordinate scheme.
    window_pos.subtract(
        webrtc::DesktopVector(desktop_config.pixel_bounds.left(),
                              desktop_config.pixel_bounds.top()));

    // We must make sure we are taking into account the fact that when we
    // find the window on the host it returns its coordinates in Density
    // Independent coordinates. We have to convert to Density Dependent
    // because InputInjector assumes Density Dependent coordinates in the
    // MouseEvent.
    window_pos.set(window_pos.x() * desktop_config.dip_to_pixel_scale,
                   window_pos.y() * desktop_config.dip_to_pixel_scale);

    // Create a new event with coordinates that are in respect to the window.
    MouseEvent modified_event(event);
    modified_event.set_x(event.x() + window_pos.x());
    modified_event.set_y(event.y() + window_pos.y());
    input_injector_->InjectMouseEvent(modified_event);
  } else {
    input_injector_->InjectMouseEvent(event);
  }
}

void SingleWindowInputInjectorMac::InjectClipboardEvent(
    const ClipboardEvent& event) {
  input_injector_->InjectClipboardEvent(event);
}

// This method finds the rectangle of the window we are streaming using
// |window_id_|. The InputInjector can then use this rectangle
// to translate the input event to coordinates of the window rather
// than the screen.
CGRect SingleWindowInputInjectorMac::FindCGRectOfWindow() {
  CGRect rect;
  CGWindowID ids[1] = {window_id_};
  base::ScopedCFTypeRef<CFArrayRef> window_id_array(
      CFArrayCreate(NULL, reinterpret_cast<const void **>(&ids), 1, NULL));

  base::ScopedCFTypeRef<CFArrayRef> window_array(
      CGWindowListCreateDescriptionFromArray(window_id_array));

  if (window_array == NULL || CFArrayGetCount(window_array) == 0) {
    // Could not find the window. It might have been closed.
    LOG(ERROR) << "Specified window to stream not found for id: "
               << window_id_;
    return CGRectNull;
  }

  // We don't use ScopedCFTypeRef for |window_array| because the
  // CFDictionaryRef returned by CFArrayGetValueAtIndex is owned by
  // window_array. The same is true of the |bounds|.
  CFDictionaryRef window =
      base::mac::CFCast<CFDictionaryRef>(
          CFArrayGetValueAtIndex(window_array, 0));

  if (CFDictionaryContainsKey(window, kCGWindowBounds)) {
    CFDictionaryRef bounds =
        base::mac::GetValueFromDictionary<CFDictionaryRef>(
            window, kCGWindowBounds);

    if (bounds) {
      if (CGRectMakeWithDictionaryRepresentation(bounds, &rect)) {
        return rect;
      }
    }
  }

  return CGRectNull;
}

scoped_ptr<InputInjector> SingleWindowInputInjector::CreateForWindow(
    webrtc::WindowId window_id,
    scoped_ptr<InputInjector> input_injector) {
  scoped_ptr<SingleWindowInputInjectorMac> injector(
      new SingleWindowInputInjectorMac(window_id, input_injector.Pass()));
  return injector.Pass();
}

}  // namespace remoting
