// SPDX-FileCopyrightText: 2025 2025 Contributors to the Media eXchange Layer project.
// SPDX-License-Identifier: Apache-2.0

#[cfg(test)]
mod tests {
    use std::collections::HashMap;

    use crate::mxlsink::imp::*;
    use crate::mxlsink::state::{Settings, group_hint_tags};
    use glib::subclass::types::ObjectSubclassType;
    use gst::{CoreError, Fraction, prelude::*};
    use gstreamer as gst;
    use mxl::flowdef::*;
    use uuid::Uuid;

    #[test]
    fn set_properties() -> Result<(), glib::Error> {
        gst::init()?;
        gst::Element::register(None, "mxlsink", gst::Rank::NONE, MxlSink::type_())
            .map_err(|e| glib::Error::new(CoreError::Failed, &e.message))?;

        let element = gst::ElementFactory::make("mxlsink")
            .build()
            .map_err(|e| glib::Error::new(CoreError::Failed, &e.message))?;

        assert_eq!(element.property::<Option<String>>("description"), None);
        assert_eq!(element.property::<Option<String>>("label"), None);
        assert_eq!(element.property::<Option<String>>("group-hint"), None);

        let element = gst::ElementFactory::make("mxlsink")
            .property("flow-id", "test_flow")
            .property("domain", "mydomain")
            .property("description", "My description")
            .property("label", "My label")
            .property("group-hint", "My Function")
            .build()
            .map_err(|e| glib::Error::new(CoreError::Failed, &e.message))?;

        assert_eq!(element.property::<String>("flow-id"), "test_flow");
        assert_eq!(element.property::<String>("domain"), "mydomain");
        assert_eq!(
            element.property::<Option<String>>("description").as_deref(),
            Some("My description")
        );
        assert_eq!(
            element.property::<Option<String>>("label").as_deref(),
            Some("My label")
        );
        assert_eq!(
            element.property::<Option<String>>("group-hint").as_deref(),
            Some("My Function")
        );
        Ok(())
    }

    #[test]
    fn group_hint_tag_fallback_and_override() {
        let tag_key = "urn:x-nmos:tag:grouphint/v1.0";

        let tags = group_hint_tags(&Settings::default(), "Video");
        assert_eq!(
            tags[tag_key],
            vec![format!("Media Function {}:Video", std::process::id())]
        );

        let settings = Settings {
            group_hint: Some("My Function".to_string()),
            ..Settings::default()
        };
        let tags = group_hint_tags(&settings, "Audio");
        assert_eq!(tags[tag_key], vec!["My Function:Audio".to_string()]);
    }

    #[test]
    #[cfg_attr(feature = "tracing", tracing_test::traced_test)]
    fn flow_def_generation() -> Result<(), glib::Error> {
        let flow_id = String::from("5fbec3b1-1b0f-417d-9059-8b94a47197ed");
        let width = 1920;
        let height = 1080;
        let framerate = Fraction::new(30000, 1001);
        let interlace_mode = InterlaceMode::Progressive;
        let colorimetry = "BT709".to_string();
        let format = "v210".to_string();
        let mut tags = HashMap::new();
        tags.insert(
            "urn:x-nmos:tag:grouphint/v1.0".to_string(),
            vec!["Media Function XYZ:Audio".to_string()],
        );
        let flow_def_details = FlowDefVideo {
            grain_rate: Rate {
                numerator: framerate.numer(),
                denominator: framerate.denom(),
            },
            frame_width: width,
            frame_height: height,
            interlace_mode,
            colorspace: colorimetry,
            components: vec![
                Component {
                    name: "Y".into(),
                    width,
                    height,
                    bit_depth: 10,
                },
                Component {
                    name: "Cb".into(),
                    width: width / 2,
                    height,
                    bit_depth: 10,
                },
                Component {
                    name: "Cr".into(),
                    width: width / 2,
                    height,
                    bit_depth: 10,
                },
            ],
        };

        let flow_def = FlowDef {
            id: Uuid::parse_str(&flow_id)
                .map_err(|_| glib::Error::new(CoreError::Failed, "Failed to parse UUID"))?,
            description: format!(
                "MXL Test Flow, 1080p{}",
                framerate.numer() / framerate.denom()
            ),
            tags,
            format: "urn:x-nmos:format:video".into(),
            label: format!(
                "MXL Test Flow, 1080p{}",
                framerate.numer() / framerate.denom()
            ),
            parents: vec![],
            media_type: format!("video/{}", format),
            details: FlowDefDetails::Video(flow_def_details),
        };

        let json = serde_json::to_value(&flow_def)
            .map_err(|_| glib::Error::new(CoreError::Failed, "Failed to convert to json"))?;

        let expected_json = serde_json::json!({
            "description": "MXL Test Flow, 1080p29",
            "id": "5fbec3b1-1b0f-417d-9059-8b94a47197ed",
            "tags": {"urn:x-nmos:tag:grouphint/v1.0": ["Media Function XYZ:Audio"]},
            "format": "urn:x-nmos:format:video",
            "label": "MXL Test Flow, 1080p29",
            "parents": [],
            "media_type": "video/v210",
            "grain_rate": {
                "numerator": 30000,
                "denominator": 1001
            },
            "frame_width": 1920,
            "frame_height": 1080,
            "interlace_mode": "progressive",
            "colorspace": "BT709",
            "components": [
                {
                    "name": "Y",
                    "width": 1920,
                    "height": 1080,
                    "bit_depth": 10
                },
                {
                    "name": "Cb",
                    "width": 960,
                    "height": 1080,
                    "bit_depth": 10
                },
                {
                    "name": "Cr",
                    "width": 960,
                    "height": 1080,
                    "bit_depth": 10
                }
            ]
        });
        println!("{:#?}", json);
        assert_eq!(json, expected_json);
        Ok(())
    }

    #[test]
    #[cfg_attr(feature = "tracing", tracing_test::traced_test)]
    #[ignore = "MXL + GStreamer integration - failing on CI but not reproducible; opt-in with cargo test -- --ignored"]
    fn valid_gray_pipeline() -> Result<(), glib::Error> {
        gst::init()?;
        gst::Element::register(
            None,
            "mxlsrc",
            gst::Rank::NONE,
            crate::mxlsrc::MxlSrc::static_type(),
        )
        .map_err(|e| glib::Error::new(CoreError::Failed, &e.message))?;
        gst::Element::register(None, "mxlsink", gst::Rank::NONE, MxlSink::type_())
            .map_err(|e| glib::Error::new(CoreError::Failed, &e.message))?;
        let pipeline = gst::Pipeline::new();
        let src = gst::ElementFactory::make("videotestsrc")
            .build()
            .map_err(|e| glib::Error::new(CoreError::Failed, &e.message))?;

        let queue1 = gst::ElementFactory::make("queue")
            .build()
            .map_err(|e| glib::Error::new(CoreError::Failed, &e.message))?;
        let convert1 = gst::ElementFactory::make("videoconvert")
            .build()
            .map_err(|e| glib::Error::new(CoreError::Failed, &e.message))?;
        let queue2 = gst::ElementFactory::make("queue")
            .build()
            .map_err(|e| glib::Error::new(CoreError::Failed, &e.message))?;

        let caps = gst::Caps::builder("video/x-raw")
            .field("format", "GRAY8")
            .build();
        let capsfilter = gst::ElementFactory::make("capsfilter")
            .property("caps", &caps)
            .build()
            .map_err(|e| glib::Error::new(CoreError::Failed, &e.message))?;

        let queue3 = gst::ElementFactory::make("queue")
            .build()
            .map_err(|e| glib::Error::new(CoreError::Failed, &e.message))?;
        let convert2 = gst::ElementFactory::make("videoconvert")
            .build()
            .map_err(|e| glib::Error::new(CoreError::Failed, &e.message))?;
        let queue4 = gst::ElementFactory::make("queue")
            .build()
            .map_err(|e| glib::Error::new(CoreError::Failed, &e.message))?;

        let sink = gst::ElementFactory::make("mxlsink")
            .property("flow-id", "7fbec3b1-1b0f-417d-9059-8b94a47197ed")
            .property("domain", "/dev/shm")
            .build()
            .map_err(|e| glib::Error::new(CoreError::Failed, &e.message))?;

        pipeline
            .add_many([
                &src,
                &queue1,
                &convert1,
                &queue2,
                &capsfilter,
                &queue3,
                &convert2,
                &queue4,
                &sink,
            ])
            .map_err(|e| glib::Error::new(CoreError::Failed, &e.message))?;
        gst::Element::link_many([
            &src,
            &queue1,
            &convert1,
            &queue2,
            &capsfilter,
            &queue3,
            &convert2,
            &queue4,
            &sink,
        ])
        .map_err(|e| glib::Error::new(CoreError::Failed, &e.message))?;
        pipeline
            .set_state(gst::State::Playing)
            .map_err(|_| glib::Error::new(CoreError::Failed, "State change failed"))?;
        let src_pad = src
            .static_pad("src")
            .ok_or(CoreError::Failed)
            .map_err(|_| glib::Error::new(CoreError::Pad, "Source pad failed"))?;
        if let Some(caps) = src_pad.current_caps() {
            println!("Negotiated caps: {}", caps);
        } else {
            println!("No negotiated caps found");
        }
        std::thread::sleep(std::time::Duration::from_millis(600));
        pipeline
            .set_state(gst::State::Null)
            .map_err(|_| glib::Error::new(CoreError::Failed, "State change failed"))?;
        Ok(())
    }
}
