package main

import (
	"fmt"
	"io"
	"os"

	"bufio"
	"encoding/base64"

	"github.com/pions/webrtc"
	"./gst"
	"github.com/pions/webrtc/pkg/ice"
)

func main() {
	reader := bufio.NewReader(os.Stdin)
	rawSd, err := reader.ReadString('\n')
	if err != nil && err != io.EOF {
		panic(err)
	}

	rawSd = "dj0wDQpvPS0gMjM3OTYzODI5NDM5OTE5NTgwIDIgSU4gSVA0IDEyNy4wLjAuMQ0Kcz0tDQp0PTAgMA0KYT1ncm91cDpCVU5ETEUgYXVkaW8gdmlkZW8NCmE9bXNpZC1zZW1hbnRpYzogV01TDQptPWF1ZGlvIDQ1MTQ5IFVEUC9UTFMvUlRQL1NBVlBGIDExMSAxMDMgMTA0IDkgMCA4IDEwNiAxMDUgMTMgMTEwIDExMiAxMTMgMTI2DQpjPUlOIElQNCAxOTIuMTY4LjEuMjMNCmE9cnRjcDo5IElOIElQNCAwLjAuMC4wDQphPWNhbmRpZGF0ZToyNzg3MDc3MDc4IDEgdWRwIDIxMjIyNjAyMjMgMTkyLjE2OC4xLjIzIDQ1MTQ5IHR5cCBob3N0IGdlbmVyYXRpb24gMCBuZXR3b3JrLWlkIDEgbmV0d29yay1jb3N0IDEwDQphPWNhbmRpZGF0ZTozOTAyNTc2NDIyIDEgdGNwIDE1MTgyODA0NDcgMTkyLjE2OC4xLjIzIDkgdHlwIGhvc3QgdGNwdHlwZSBhY3RpdmUgZ2VuZXJhdGlvbiAwIG5ldHdvcmstaWQgMSBuZXR3b3JrLWNvc3QgMTANCmE9aWNlLXVmcmFnOktla08NCmE9aWNlLXB3ZDpLa3VhNE5XV2dBaUlpMHBrQjVCSWNSTXENCmE9aWNlLW9wdGlvbnM6dHJpY2tsZQ0KYT1maW5nZXJwcmludDpzaGEtMjU2IEMxOjcwOjRFOjc3OjZEOjU0OkE0OkU5OkUxOkZEOjYzOjgyOkRFOjlGOjFGOjM2OkNFOjA5OjNEOjNBOjdGOkRBOjlDOkY2OjgzOjJEOjE1OjRDOjVCOkJBOkU4OkE1DQphPXNldHVwOmFjdHBhc3MNCmE9bWlkOmF1ZGlvDQphPWV4dG1hcDoxIHVybjppZXRmOnBhcmFtczpydHAtaGRyZXh0OnNzcmMtYXVkaW8tbGV2ZWwNCmE9cmVjdm9ubHkNCmE9cnRjcC1tdXgNCmE9cnRwbWFwOjExMSBvcHVzLzQ4MDAwLzINCmE9cnRjcC1mYjoxMTEgdHJhbnNwb3J0LWNjDQphPWZtdHA6MTExIG1pbnB0aW1lPTEwO3VzZWluYmFuZGZlYz0xDQphPXJ0cG1hcDoxMDMgSVNBQy8xNjAwMA0KYT1ydHBtYXA6MTA0IElTQUMvMzIwMDANCmE9cnRwbWFwOjkgRzcyMi84MDAwDQphPXJ0cG1hcDowIFBDTVUvODAwMA0KYT1ydHBtYXA6OCBQQ01BLzgwMDANCmE9cnRwbWFwOjEwNiBDTi8zMjAwMA0KYT1ydHBtYXA6MTA1IENOLzE2MDAwDQphPXJ0cG1hcDoxMyBDTi84MDAwDQphPXJ0cG1hcDoxMTAgdGVsZXBob25lLWV2ZW50LzQ4MDAwDQphPXJ0cG1hcDoxMTIgdGVsZXBob25lLWV2ZW50LzMyMDAwDQphPXJ0cG1hcDoxMTMgdGVsZXBob25lLWV2ZW50LzE2MDAwDQphPXJ0cG1hcDoxMjYgdGVsZXBob25lLWV2ZW50LzgwMDANCm09dmlkZW8gMzY0NjcgVURQL1RMUy9SVFAvU0FWUEYgOTYgOTcgOTggOTkgMTAwIDEwMSAxMDIgMTI0IDEyNyAxMjMgMTI1DQpjPUlOIElQNCAxOTIuMTY4LjEuMjMNCmE9cnRjcDo5IElOIElQNCAwLjAuMC4wDQphPWNhbmRpZGF0ZToyNzg3MDc3MDc4IDEgdWRwIDIxMjIyNjAyMjMgMTkyLjE2OC4xLjIzIDM2NDY3IHR5cCBob3N0IGdlbmVyYXRpb24gMCBuZXR3b3JrLWlkIDEgbmV0d29yay1jb3N0IDEwDQphPWNhbmRpZGF0ZTozOTAyNTc2NDIyIDEgdGNwIDE1MTgyODA0NDcgMTkyLjE2OC4xLjIzIDkgdHlwIGhvc3QgdGNwdHlwZSBhY3RpdmUgZ2VuZXJhdGlvbiAwIG5ldHdvcmstaWQgMSBuZXR3b3JrLWNvc3QgMTANCmE9aWNlLXVmcmFnOktla08NCmE9aWNlLXB3ZDpLa3VhNE5XV2dBaUlpMHBrQjVCSWNSTXENCmE9aWNlLW9wdGlvbnM6dHJpY2tsZQ0KYT1maW5nZXJwcmludDpzaGEtMjU2IEMxOjcwOjRFOjc3OjZEOjU0OkE0OkU5OkUxOkZEOjYzOjgyOkRFOjlGOjFGOjM2OkNFOjA5OjNEOjNBOjdGOkRBOjlDOkY2OjgzOjJEOjE1OjRDOjVCOkJBOkU4OkE1DQphPXNldHVwOmFjdHBhc3MNCmE9bWlkOnZpZGVvDQphPWV4dG1hcDoyIHVybjppZXRmOnBhcmFtczpydHAtaGRyZXh0OnRvZmZzZXQNCmE9ZXh0bWFwOjMgaHR0cDovL3d3dy53ZWJydGMub3JnL2V4cGVyaW1lbnRzL3J0cC1oZHJleHQvYWJzLXNlbmQtdGltZQ0KYT1leHRtYXA6NCB1cm46M2dwcDp2aWRlby1vcmllbnRhdGlvbg0KYT1leHRtYXA6NSBodHRwOi8vd3d3LmlldGYub3JnL2lkL2RyYWZ0LWhvbG1lci1ybWNhdC10cmFuc3BvcnQtd2lkZS1jYy1leHRlbnNpb25zLTAxDQphPWV4dG1hcDo2IGh0dHA6Ly93d3cud2VicnRjLm9yZy9leHBlcmltZW50cy9ydHAtaGRyZXh0L3BsYXlvdXQtZGVsYXkNCmE9ZXh0bWFwOjcgaHR0cDovL3d3dy53ZWJydGMub3JnL2V4cGVyaW1lbnRzL3J0cC1oZHJleHQvdmlkZW8tY29udGVudC10eXBlDQphPWV4dG1hcDo4IGh0dHA6Ly93d3cud2VicnRjLm9yZy9leHBlcmltZW50cy9ydHAtaGRyZXh0L3ZpZGVvLXRpbWluZw0KYT1yZWN2b25seQ0KYT1ydGNwLW11eA0KYT1ydGNwLXJzaXplDQphPXJ0cG1hcDo5NiBWUDgvOTAwMDANCmE9cnRjcC1mYjo5NiBnb29nLXJlbWINCmE9cnRjcC1mYjo5NiB0cmFuc3BvcnQtY2MNCmE9cnRjcC1mYjo5NiBjY20gZmlyDQphPXJ0Y3AtZmI6OTYgbmFjaw0KYT1ydGNwLWZiOjk2IG5hY2sgcGxpDQphPXJ0cG1hcDo5NyBydHgvOTAwMDANCmE9Zm10cDo5NyBhcHQ9OTYNCmE9cnRwbWFwOjk4IFZQOS85MDAwMA0KYT1ydGNwLWZiOjk4IGdvb2ctcmVtYg0KYT1ydGNwLWZiOjk4IHRyYW5zcG9ydC1jYw0KYT1ydGNwLWZiOjk4IGNjbSBmaXINCmE9cnRjcC1mYjo5OCBuYWNrDQphPXJ0Y3AtZmI6OTggbmFjayBwbGkNCmE9cnRwbWFwOjk5IHJ0eC85MDAwMA0KYT1mbXRwOjk5IGFwdD05OA0KYT1ydHBtYXA6MTAwIEgyNjQvOTAwMDANCmE9cnRjcC1mYjoxMDAgZ29vZy1yZW1iDQphPXJ0Y3AtZmI6MTAwIHRyYW5zcG9ydC1jYw0KYT1ydGNwLWZiOjEwMCBjY20gZmlyDQphPXJ0Y3AtZmI6MTAwIG5hY2sNCmE9cnRjcC1mYjoxMDAgbmFjayBwbGkNCmE9Zm10cDoxMDAgbGV2ZWwtYXN5bW1ldHJ5LWFsbG93ZWQ9MTtwYWNrZXRpemF0aW9uLW1vZGU9MTtwcm9maWxlLWxldmVsLWlkPTQyMDAxZg0KYT1ydHBtYXA6MTAxIHJ0eC85MDAwMA0KYT1mbXRwOjEwMSBhcHQ9MTAwDQphPXJ0cG1hcDoxMDIgSDI2NC85MDAwMA0KYT1ydGNwLWZiOjEwMiBnb29nLXJlbWINCmE9cnRjcC1mYjoxMDIgdHJhbnNwb3J0LWNjDQphPXJ0Y3AtZmI6MTAyIGNjbSBmaXINCmE9cnRjcC1mYjoxMDIgbmFjaw0KYT1ydGNwLWZiOjEwMiBuYWNrIHBsaQ0KYT1mbXRwOjEwMiBsZXZlbC1hc3ltbWV0cnktYWxsb3dlZD0xO3BhY2tldGl6YXRpb24tbW9kZT0xO3Byb2ZpbGUtbGV2ZWwtaWQ9NDJlMDFmDQphPXJ0cG1hcDoxMjQgcnR4LzkwMDAwDQphPWZtdHA6MTI0IGFwdD0xMDINCmE9cnRwbWFwOjEyNyByZWQvOTAwMDANCmE9cnRwbWFwOjEyMyBydHgvOTAwMDANCmE9Zm10cDoxMjMgYXB0PTEyNw0KYT1ydHBtYXA6MTI1IHVscGZlYy85MDAwMA0K"

	fmt.Println("")
	sd, err := base64.StdEncoding.DecodeString(rawSd)
	if err != nil {
		panic(err)
	}

	/* Everything below is the pion-WebRTC API, thanks for using it! */

	// Setup the codecs you want to use.
	// We'll use the default ones but you can also define your own
	webrtc.RegisterDefaultCodecs()

	// Create a new RTCPeerConnection
	peerConnection, err := webrtc.New(webrtc.RTCConfiguration{
		ICEServers: []webrtc.RTCICEServer{
			{
				URLs: []string{"stun:stun.l.google.com:19302"},
			},
		},
	})
	if err != nil {
		panic(err)
	}

	// Set the handler for ICE connection state
	// This will notify you when the peer has connected/disconnected
	peerConnection.OnICEConnectionStateChange = func(connectionState ice.ConnectionState) {
		fmt.Printf("Connection State has changed %s \n", connectionState.String())
	}

	// Create a audio track
	opusTrack, err := peerConnection.NewRTCTrack(webrtc.DefaultPayloadTypeOpus, "audio", "pion1")
	if err != nil {
		panic(err)
	}
	_, err = peerConnection.AddTrack(opusTrack)
	if err != nil {
		panic(err)
	}

	// Create a video track
	vp8Track, err := peerConnection.NewRTCTrack(webrtc.DefaultPayloadTypeVP8, "video", "pion2")
	if err != nil {
		panic(err)
	}
	_, err = peerConnection.AddTrack(vp8Track)
	if err != nil {
		panic(err)
	}

	// Set the remote SessionDescription
	offer := webrtc.RTCSessionDescription{
		Type: webrtc.RTCSdpTypeOffer,
		Sdp:  string(sd),
	}
	if err := peerConnection.SetRemoteDescription(offer); err != nil {
		panic(err)
	}

	// Sets the LocalDescription, and starts our UDP listeners
	answer, err := peerConnection.CreateAnswer(nil)
	if err != nil {
		panic(err)
	}

	// Get the LocalDescription and take it to base64 so we can paste in browser
	fmt.Println(base64.StdEncoding.EncodeToString([]byte(answer.Sdp)))

	// Start pushing buffers on these tracks
	gst.CreatePipeline(webrtc.Opus, opusTrack.Samples).Start()
	gst.CreatePipeline("test", vp8Track.Samples).Start()
	select {}
}
