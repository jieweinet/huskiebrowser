// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/common/ui/confirmation_alert/confirmation_alert_view_controller.h"

#include "base/check.h"
#import "ios/chrome/common/ui/colors/semantic_color_names.h"
#import "ios/chrome/common/ui/confirmation_alert/confirmation_alert_action_handler.h"
#import "ios/chrome/common/ui/util/button_util.h"
#import "ios/chrome/common/ui/util/constraints_ui_util.h"
#include "ios/chrome/common/ui/util/dynamic_type_util.h"
#import "ios/chrome/common/ui/util/image_util.h"
#import "ios/chrome/common/ui/util/pointer_interaction_util.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

NSString* const kConfirmationAlertMoreInfoAccessibilityIdentifier =
    @"kConfirmationAlertMoreInfoAccessibilityIdentifier";
NSString* const kConfirmationAlertTitleAccessibilityIdentifier =
    @"kConfirmationAlertTitleAccessibilityIdentifier";
NSString* const kConfirmationAlertSubtitleAccessibilityIdentifier =
    @"kConfirmationAlertSubtitleAccessibilityIdentifier";
NSString* const kConfirmationAlertPrimaryActionAccessibilityIdentifier =
    @"kConfirmationAlertPrimaryActionAccessibilityIdentifier";
NSString* const kConfirmationAlertSecondaryActionAccessibilityIdentifier =
    @"kConfirmationAlertSecondaryActionAccessibilityIdentifier";
NSString* const kConfirmationAlertTertiaryActionAccessibilityIdentifier =
    @"kConfirmationAlertTertiaryActionAccessibilityIdentifier";
NSString* const kConfirmationAlertBarPrimaryActionAccessibilityIdentifier =
    @"kConfirmationAlertBarPrimaryActionAccessibilityIdentifier";

namespace {

constexpr CGFloat kScrollViewBottomInsets = 20;
constexpr CGFloat kStackViewSpacing = 8;
constexpr CGFloat kStackViewSpacingAfterIllustration = 27;
constexpr CGFloat kGeneratedImagePadding = 20;
// The multiplier used when in regular horizontal size class.
constexpr CGFloat kSafeAreaMultiplier = 0.8;
constexpr CGFloat kButtonMaxWidth = 327;
constexpr CGFloat kContentMaxWidth = 500;

}  // namespace

@interface ConfirmationAlertViewController () <UIToolbarDelegate>

// Container view that will wrap the views making up the content.
@property(nonatomic, strong) UIStackView* stackView;

// References to the UI properties that need to be updated when the trait
// collection changes.
@property(nonatomic, strong) UIButton* primaryActionButton;
@property(nonatomic, strong) UIButton* secondaryActionButton;
@property(nonatomic, strong) UIButton* tertiaryActionButton;
@property(nonatomic, strong) UIToolbar* topToolbar;
@property(nonatomic, strong) NSArray* regularHeightToolbarItems;
@property(nonatomic, strong) NSArray* compactHeightToolbarItems;
@property(nonatomic, strong) UIImageView* imageView;
// Constraints.
@property(nonatomic, strong)
    NSArray<NSLayoutConstraint*>* compactWidthConstraints;
@property(nonatomic, strong)
    NSArray<NSLayoutConstraint*>* regularWidthConstraints;
@property(nonatomic, strong)
    NSLayoutConstraint* regularHeightScrollViewBottomVerticalConstraint;
@property(nonatomic, strong)
    NSLayoutConstraint* compactHeightScrollViewBottomVerticalConstraint;
@property(nonatomic, strong)
    NSLayoutConstraint* buttonStackViewBottomVerticalConstraint;
@end

@implementation ConfirmationAlertViewController

#pragma mark - Public

- (instancetype)init {
  self = [super init];
  if (self) {
    _customSpacingAfterImage = kStackViewSpacingAfterIllustration;
    _showDismissBarButton = YES;
    _dismissBarButtonSystemItem = UIBarButtonSystemItemDone;
  }
  return self;
}

- (void)viewDidLoad {
  [super viewDidLoad];

  self.view.backgroundColor = [UIColor colorNamed:kBackgroundColor];

  if (self.hasTopToolbar) {
    self.topToolbar = [self createTopToolbar];
    [self.view addSubview:self.topToolbar];
  }

  self.imageView = [self createImageView];
  UILabel* title = [self createTitleLabel];
  UILabel* subtitle = [self createSubtitleLabel];

  NSArray* stackSubviews = @[ self.imageView, title, subtitle ];
  self.stackView = [self createStackViewWithArrangedSubviews:stackSubviews];

  UIScrollView* scrollView = [self createScrollView];
  [scrollView addSubview:self.stackView];
  [self.view addSubview:scrollView];

  self.view.preservesSuperviewLayoutMargins = YES;
  UILayoutGuide* margins = self.view.layoutMarginsGuide;

  if (self.hasTopToolbar) {
    // Toolbar constraints to the top.
    AddSameConstraintsToSides(
        self.topToolbar, self.view.safeAreaLayoutGuide,
        LayoutSides::kTrailing | LayoutSides::kTop | LayoutSides::kLeading);
  }

  // Scroll View constraints to the height of its content. Can be overridden.
  NSLayoutConstraint* heightConstraint = [scrollView.heightAnchor
      constraintEqualToAnchor:scrollView.contentLayoutGuide.heightAnchor];
  // UILayoutPriorityDefaultHigh is the default priority for content
  // compression. Setting this lower avoids compressing the content of the
  // scroll view.
  heightConstraint.priority = UILayoutPriorityDefaultHigh - 1;
  heightConstraint.active = YES;

  if (!self.topAlignedLayout) {
    // Scroll View constraint to the vertical center. Can be overridden.
    NSLayoutConstraint* centerYConstraint = [scrollView.centerYAnchor
        constraintEqualToAnchor:margins.centerYAnchor];
    // This needs to be lower than the height constraint, so it's deprioritized.
    // If this breaks, the scroll view is still constrained to the top toolbar
    // and the bottom safe area or button.
    centerYConstraint.priority = heightConstraint.priority - 1;
    centerYConstraint.active = YES;
  }

  // Constraint the content of the scroll view to the size of the stack view
  // with some bottom margin space in between the two. This defines the content
  // area.
  NSLayoutConstraint* stackViewWidth =
      [self.stackView.widthAnchor constraintEqualToConstant:kContentMaxWidth];
  stackViewWidth.priority = UILayoutPriorityRequired - 1;
  [NSLayoutConstraint activateConstraints:@[
    [self.stackView.leadingAnchor
        constraintGreaterThanOrEqualToAnchor:scrollView.leadingAnchor],
    [self.stackView.trailingAnchor
        constraintLessThanOrEqualToAnchor:scrollView.trailingAnchor],
    [self.stackView.topAnchor constraintEqualToAnchor:scrollView.topAnchor],
    [self.stackView.bottomAnchor
        constraintEqualToAnchor:scrollView.bottomAnchor
                       constant:-kScrollViewBottomInsets],
    [self.stackView.centerXAnchor
        constraintEqualToAnchor:scrollView.centerXAnchor],
    stackViewWidth,
  ]];

  // Disable horizontal scrolling and constraint the content size to the scroll
  // view size.
  NSLayoutConstraint* scrollViewWidth = [scrollView.widthAnchor
      constraintEqualToAnchor:scrollView.contentLayoutGuide.widthAnchor];
  scrollViewWidth.priority = UILayoutPriorityDefaultHigh;
  scrollViewWidth.active = YES;

  [scrollView.centerXAnchor constraintEqualToAnchor:margins.centerXAnchor]
      .active = YES;

  // Width Scroll View constraint. It changes based on the size class.
  self.compactWidthConstraints = @[
    [scrollView.widthAnchor constraintEqualToAnchor:margins.widthAnchor],
  ];
  self.regularWidthConstraints = @[
    [scrollView.widthAnchor constraintEqualToAnchor:margins.widthAnchor
                                         multiplier:kSafeAreaMultiplier],
  ];

  // The bottom anchor for the scroll view.
  NSLayoutYAxisAnchor* scrollViewBottomAnchor =
      self.view.safeAreaLayoutGuide.bottomAnchor;
  BOOL hasActionButton = self.primaryActionString ||
                         self.secondaryActionString ||
                         self.tertiaryActionString;
  if (hasActionButton) {
    UIStackView* actionStackView = [[UIStackView alloc] init];
    actionStackView.alignment = UIStackViewAlignmentFill;
    actionStackView.axis = UILayoutConstraintAxisVertical;
    actionStackView.translatesAutoresizingMaskIntoConstraints = NO;

    if (self.primaryActionString) {
      self.primaryActionButton = [self createPrimaryActionButton];
      [actionStackView addArrangedSubview:self.primaryActionButton];
    }

    if (self.secondaryActionString) {
      self.secondaryActionButton = [self createSecondaryActionButton];
      [actionStackView addArrangedSubview:self.secondaryActionButton];
    }

    if (self.tertiaryActionString) {
      self.tertiaryActionButton = [self createTertiaryButton];
      [actionStackView addArrangedSubview:self.tertiaryActionButton];
    }

    [self.view addSubview:actionStackView];
    self.buttonStackViewBottomVerticalConstraint = [actionStackView.bottomAnchor
        constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor];
    // Add a low priority width constraints to make sure that the buttons are
    // taking as much width as they can.
    NSLayoutConstraint* lowPriorityWidthConstraint =
        [actionStackView.widthAnchor constraintEqualToConstant:kButtonMaxWidth];
    lowPriorityWidthConstraint.priority = UILayoutPriorityDefaultHigh;

    [NSLayoutConstraint activateConstraints:@[
      [actionStackView.leadingAnchor
          constraintGreaterThanOrEqualToAnchor:scrollView.leadingAnchor],
      [actionStackView.trailingAnchor
          constraintLessThanOrEqualToAnchor:scrollView.trailingAnchor],
      self.buttonStackViewBottomVerticalConstraint,
      [actionStackView.centerXAnchor
          constraintEqualToAnchor:self.view.centerXAnchor],
      [actionStackView.widthAnchor
          constraintLessThanOrEqualToConstant:kButtonMaxWidth],
      lowPriorityWidthConstraint
    ]];
    scrollViewBottomAnchor = actionStackView.topAnchor;
  }

  self.regularHeightScrollViewBottomVerticalConstraint =
      [scrollView.bottomAnchor
          constraintLessThanOrEqualToAnchor:scrollViewBottomAnchor];
  self.compactHeightScrollViewBottomVerticalConstraint =
      [scrollView.bottomAnchor
          constraintLessThanOrEqualToAnchor:scrollViewBottomAnchor];

  if (self.alwaysShowImage && self.primaryActionString) {
    // If we always want to show the image, then it means we must hide the
    // button when in compact height mode - meaning we have to constraint the
    // scrollview's bottom to the safeArea's bottom.
    self.compactHeightScrollViewBottomVerticalConstraint =
        [scrollView.bottomAnchor
            constraintLessThanOrEqualToAnchor:self.view.safeAreaLayoutGuide
                                                  .bottomAnchor];
  }

  NSLayoutYAxisAnchor* scrollViewTopAnchor;
  CGFloat scrollViewTopConstant = 0;
  if (self.hasTopToolbar) {
    scrollViewTopAnchor = self.topToolbar.bottomAnchor;
  } else {
    scrollViewTopAnchor = self.view.safeAreaLayoutGuide.topAnchor;
    scrollViewTopConstant = self.customSpacingBeforeImageIfNoToolbar;
  }
  if (self.topAlignedLayout) {
    [scrollView.topAnchor constraintEqualToAnchor:scrollViewTopAnchor
                                         constant:scrollViewTopConstant]
        .active = YES;
  } else {
    [scrollView.topAnchor
        constraintGreaterThanOrEqualToAnchor:scrollViewTopAnchor
                                    constant:scrollViewTopConstant]
        .active = YES;
  }

  if (!self.imageHasFixedSize) {
    // Constrain the image to the scroll view size and its aspect ratio.
    [self.imageView
        setContentCompressionResistancePriority:UILayoutPriorityDefaultLow
                                        forAxis:
                                            UILayoutConstraintAxisHorizontal];
    [self.imageView
        setContentCompressionResistancePriority:UILayoutPriorityDefaultLow
                                        forAxis:UILayoutConstraintAxisVertical];
    CGFloat imageAspectRatio =
        self.imageView.image.size.width / self.imageView.image.size.height;

    [NSLayoutConstraint activateConstraints:@[
      [self.imageView.widthAnchor
          constraintEqualToAnchor:self.imageView.heightAnchor
                       multiplier:imageAspectRatio],
    ]];
  }
}

- (void)traitCollectionDidChange:(UITraitCollection*)previousTraitCollection {
  [super traitCollectionDidChange:previousTraitCollection];

  // Update fonts for specific content sizes.
  if (previousTraitCollection.preferredContentSizeCategory !=
      self.traitCollection.preferredContentSizeCategory) {
    self.primaryActionButton.titleLabel.font =
        PreferredFontForTextStyleWithMaxCategory(
            UIFontTextStyleHeadline,
            self.traitCollection.preferredContentSizeCategory,
            UIContentSizeCategoryExtraExtraExtraLarge);
  }

  // Update constraints for different size classes.
  BOOL hasNewHorizontalSizeClass =
      previousTraitCollection.horizontalSizeClass !=
      self.traitCollection.horizontalSizeClass;
  BOOL hasNewVerticalSizeClass = previousTraitCollection.verticalSizeClass !=
                                 self.traitCollection.verticalSizeClass;

  if (hasNewHorizontalSizeClass || hasNewVerticalSizeClass) {
    [self.view setNeedsUpdateConstraints];
  }
}

- (void)viewSafeAreaInsetsDidChange {
  [super viewSafeAreaInsetsDidChange];
  [self.view setNeedsUpdateConstraints];
}

- (void)viewLayoutMarginsDidChange {
  [super viewLayoutMarginsDidChange];
  [self.view setNeedsUpdateConstraints];
}

- (void)updateViewConstraints {
  CGFloat marginValue =
      self.view.layoutMargins.left - self.view.safeAreaInsets.left;
  if (!self.secondaryActionString) {
    // Do not add margin padding between the bottom button and the containing
    // view if the primary button is the bottom button to allow for more visual
    // spacing between the content and the button. The secondary button has a
    // transparent background so the visual spacing already exists.
    self.buttonStackViewBottomVerticalConstraint.constant = -marginValue;
  }
  if (self.traitCollection.horizontalSizeClass ==
      UIUserInterfaceSizeClassCompact) {
    [NSLayoutConstraint deactivateConstraints:self.regularWidthConstraints];
    [NSLayoutConstraint activateConstraints:self.compactWidthConstraints];
  } else {
    [NSLayoutConstraint deactivateConstraints:self.compactWidthConstraints];
    [NSLayoutConstraint activateConstraints:self.regularWidthConstraints];
  }

  BOOL isVerticalCompact =
      self.traitCollection.verticalSizeClass == UIUserInterfaceSizeClassCompact;

  NSLayoutConstraint* oldBottomConstraint;
  NSLayoutConstraint* newBottomConstraint;
  if (isVerticalCompact) {
    oldBottomConstraint = self.regularHeightScrollViewBottomVerticalConstraint;
    newBottomConstraint = self.compactHeightScrollViewBottomVerticalConstraint;

    // Use setItems:animated method instead of setting the items property, as
    // that causes issues with the Done button. See crbug.com/1082723
    [self.topToolbar setItems:self.compactHeightToolbarItems animated:YES];
  } else {
    oldBottomConstraint = self.compactHeightScrollViewBottomVerticalConstraint;
    newBottomConstraint = self.regularHeightScrollViewBottomVerticalConstraint;

    // Use setItems:animated method instead of setting the items property, as
    // that causes issues with the Done button. See crbug.com/1082723
    [self.topToolbar setItems:self.regularHeightToolbarItems animated:YES];
  }

  if (!self.secondaryActionString) {
    newBottomConstraint.constant = -marginValue;
  }
  [NSLayoutConstraint deactivateConstraints:@[ oldBottomConstraint ]];
  [NSLayoutConstraint activateConstraints:@[ newBottomConstraint ]];

  if (self.alwaysShowImage) {
    // Update the primary action button visibility.
    [self.primaryActionButton setHidden:isVerticalCompact];
  } else {
    [self.imageView setHidden:isVerticalCompact];
  }

  // Allow toolbar to update its height based on new layout.
  [self.topToolbar invalidateIntrinsicContentSize];

  [super updateViewConstraints];
}

- (UIImage*)content {
  UIEdgeInsets padding =
      UIEdgeInsetsMake(kGeneratedImagePadding, kGeneratedImagePadding,
                       kGeneratedImagePadding, kGeneratedImagePadding);
  return ImageFromView(self.stackView, self.view.backgroundColor, padding);
}

#pragma mark - UIToolbarDelegate

- (UIBarPosition)positionForBar:(id<UIBarPositioning>)bar {
  return UIBarPositionTopAttached;
}

#pragma mark - Private

// Handle taps on the dismiss button.
- (void)didTapDismissBarButton {
  DCHECK(self.showDismissBarButton);
  if ([self.actionHandler
          respondsToSelector:@selector(confirmationAlertDismissAction)]) {
    [self.actionHandler confirmationAlertDismissAction];
  }
}

// Handle taps on the help button.
- (void)didTapHelpButton {
  if ([self.actionHandler
          respondsToSelector:@selector(confirmationAlertLearnMoreAction)]) {
    [self.actionHandler confirmationAlertLearnMoreAction];
  }
}

// Handle taps on the primary action button.
- (void)didTapPrimaryActionButton {
  [self.actionHandler confirmationAlertPrimaryAction];
}

// Handle taps on the secondary action button
- (void)didTapSecondaryActionButton {
  DCHECK(self.secondaryActionString);
  if ([self.actionHandler
          respondsToSelector:@selector(confirmationAlertSecondaryAction)]) {
    [self.actionHandler confirmationAlertSecondaryAction];
  }
}

- (void)didTapTertiaryActionButton {
  DCHECK(self.tertiaryActionString);
  if ([self.actionHandler
          respondsToSelector:@selector(confirmationAlertTertiaryAction)]) {
    [self.actionHandler confirmationAlertTertiaryAction];
  }
}

// Helper to create the top toolbar.
- (UIToolbar*)createTopToolbar {
  UIToolbar* topToolbar = [[UIToolbar alloc] init];
  topToolbar.translucent = NO;
  [topToolbar setShadowImage:[[UIImage alloc] init]
          forToolbarPosition:UIBarPositionAny];
  [topToolbar setBarTintColor:[UIColor colorNamed:kBackgroundColor]];
  topToolbar.delegate = self;

  NSMutableArray* regularHeightItems = [[NSMutableArray alloc] init];
  NSMutableArray* compactHeightItems = [[NSMutableArray alloc] init];
  if (self.helpButtonAvailable) {
    UIBarButtonItem* helpButton =
        [[UIBarButtonItem alloc] initWithImage:[UIImage imageNamed:@"help_icon"]
                                         style:UIBarButtonItemStylePlain
                                        target:self
                                        action:@selector(didTapHelpButton)];
    [regularHeightItems addObject:helpButton];
    [compactHeightItems addObject:helpButton];

    if (self.helpButtonAccessibilityLabel) {
      helpButton.isAccessibilityElement = YES;
      helpButton.accessibilityLabel = self.helpButtonAccessibilityLabel;
    }

    helpButton.accessibilityIdentifier =
        kConfirmationAlertMoreInfoAccessibilityIdentifier;
    // Set the help button as the left button item so it can be used as a
    // popover anchor.
    _helpButton = helpButton;
  }

  if (self.alwaysShowImage && self.primaryActionString) {
    if (self.helpButtonAvailable) {
      // Add margin with help button.
      UIBarButtonItem* fixedSpacer = [[UIBarButtonItem alloc]
          initWithBarButtonSystemItem:UIBarButtonSystemItemFixedSpace
                               target:nil
                               action:nil];
      fixedSpacer.width = 15.0f;
      [compactHeightItems addObject:fixedSpacer];
    }

    UIBarButtonItem* primaryActionBarButton = [[UIBarButtonItem alloc]
        initWithBarButtonSystemItem:self.primaryActionBarButtonStyle
                             target:self
                             action:@selector(didTapPrimaryActionButton)];
    primaryActionBarButton.accessibilityIdentifier =
        kConfirmationAlertBarPrimaryActionAccessibilityIdentifier;

    // Only shows up in constraint height mode.
    [compactHeightItems addObject:primaryActionBarButton];
  }

  UIBarButtonItem* spacer = [[UIBarButtonItem alloc]
      initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
                           target:nil
                           action:nil];
  [regularHeightItems addObject:spacer];
  [compactHeightItems addObject:spacer];

  if (self.showDismissBarButton) {
    UIBarButtonItem* dismissButton = [[UIBarButtonItem alloc]
        initWithBarButtonSystemItem:self.dismissBarButtonSystemItem
                             target:self
                             action:@selector(didTapDismissBarButton)];
    [regularHeightItems addObject:dismissButton];
    [compactHeightItems addObject:dismissButton];
  }

  topToolbar.translatesAutoresizingMaskIntoConstraints = NO;

  self.regularHeightToolbarItems = regularHeightItems;
  self.compactHeightToolbarItems = compactHeightItems;

  return topToolbar;
}

// Helper to create the image view.
- (UIImageView*)createImageView {
  UIImageView* imageView = [[UIImageView alloc] initWithImage:self.image];
  imageView.contentMode = UIViewContentModeScaleAspectFit;

  if (self.imageAccessibilityLabel) {
    imageView.isAccessibilityElement = YES;
    imageView.accessibilityLabel = self.imageAccessibilityLabel;
  }

  imageView.translatesAutoresizingMaskIntoConstraints = NO;
  return imageView;
}

// Helper to create the title label.
- (UILabel*)createTitleLabel {
  if (!self.titleTextStyle) {
    self.titleTextStyle = UIFontTextStyleTitle1;
  }
  UILabel* title = [[UILabel alloc] init];
  title.numberOfLines = 0;
  UIFontDescriptor* descriptor = [UIFontDescriptor
      preferredFontDescriptorWithTextStyle:self.titleTextStyle];
  UIFont* font = [UIFont systemFontOfSize:descriptor.pointSize
                                   weight:UIFontWeightBold];
  UIFontMetrics* fontMetrics =
      [UIFontMetrics metricsForTextStyle:self.titleTextStyle];
  title.font = [fontMetrics scaledFontForFont:font];
  title.textColor = [UIColor colorNamed:kTextPrimaryColor];
  title.text = self.titleString;
  title.textAlignment = NSTextAlignmentCenter;
  title.translatesAutoresizingMaskIntoConstraints = NO;
  title.adjustsFontForContentSizeCategory = YES;
  title.accessibilityIdentifier =
      kConfirmationAlertTitleAccessibilityIdentifier;
  return title;
}

// Helper to create the subtitle label.
- (UILabel*)createSubtitleLabel {
  UILabel* subtitle = [[UILabel alloc] init];
  subtitle.font = [UIFont preferredFontForTextStyle:UIFontTextStyleBody];
  subtitle.numberOfLines = 0;
  subtitle.textColor = [UIColor colorNamed:kTextSecondaryColor];
  subtitle.text = self.subtitleString;
  subtitle.textAlignment = NSTextAlignmentCenter;
  subtitle.translatesAutoresizingMaskIntoConstraints = NO;
  subtitle.adjustsFontForContentSizeCategory = YES;
  subtitle.accessibilityIdentifier =
      kConfirmationAlertSubtitleAccessibilityIdentifier;
  return subtitle;
}

- (BOOL)hasTopToolbar {
  return self.helpButtonAvailable || self.showDismissBarButton;
}

// Helper to create the scroll view.
- (UIScrollView*)createScrollView {
  UIScrollView* scrollView = [[UIScrollView alloc] init];
  scrollView.alwaysBounceVertical = NO;
  scrollView.showsHorizontalScrollIndicator = NO;
  scrollView.translatesAutoresizingMaskIntoConstraints = NO;
  return scrollView;
}

// Helper to create the stack view.
- (UIStackView*)createStackViewWithArrangedSubviews:
    (NSArray<UIView*>*)subviews {
  UIStackView* stackView =
      [[UIStackView alloc] initWithArrangedSubviews:subviews];
  [stackView setCustomSpacing:self.customSpacingAfterImage
                    afterView:self.imageView];

  if (self.imageHasFixedSize) {
    stackView.alignment = UIStackViewAlignmentCenter;
  } else {
    stackView.alignment = UIStackViewAlignmentFill;
  }

  stackView.axis = UILayoutConstraintAxisVertical;
  stackView.translatesAutoresizingMaskIntoConstraints = NO;
  stackView.spacing = kStackViewSpacing;
  return stackView;
}

// Helper to create the primary action button.
- (UIButton*)createPrimaryActionButton {
  UIButton* primaryActionButton = PrimaryActionButton(YES);
  [primaryActionButton addTarget:self
                          action:@selector(didTapPrimaryActionButton)
                forControlEvents:UIControlEventTouchUpInside];
  [primaryActionButton setTitle:self.primaryActionString
                       forState:UIControlStateNormal];
  primaryActionButton.accessibilityIdentifier =
      kConfirmationAlertPrimaryActionAccessibilityIdentifier;
  primaryActionButton.titleLabel.adjustsFontSizeToFitWidth = YES;

  return primaryActionButton;
}

// Helper to create the primary action button.
- (UIButton*)createSecondaryActionButton {
  DCHECK(self.secondaryActionString);
  UIButton* secondaryActionButton =
      [UIButton buttonWithType:UIButtonTypeSystem];
  [secondaryActionButton addTarget:self
                            action:@selector(didTapSecondaryActionButton)
                  forControlEvents:UIControlEventTouchUpInside];
  [secondaryActionButton setTitle:self.secondaryActionString
                         forState:UIControlStateNormal];
  secondaryActionButton.contentEdgeInsets =
      UIEdgeInsetsMake(kButtonVerticalInsets, 0, kButtonVerticalInsets, 0);
  [secondaryActionButton setBackgroundColor:[UIColor clearColor]];
  UIColor* titleColor = [UIColor colorNamed:kBlueColor];
  [secondaryActionButton setTitleColor:titleColor
                              forState:UIControlStateNormal];
  secondaryActionButton.titleLabel.font =
      [UIFont preferredFontForTextStyle:UIFontTextStyleBody];
  secondaryActionButton.titleLabel.adjustsFontForContentSizeCategory = NO;
  secondaryActionButton.translatesAutoresizingMaskIntoConstraints = NO;
  secondaryActionButton.accessibilityIdentifier =
      kConfirmationAlertSecondaryActionAccessibilityIdentifier;
  secondaryActionButton.titleLabel.adjustsFontSizeToFitWidth = YES;

  secondaryActionButton.pointerInteractionEnabled = YES;
  secondaryActionButton.pointerStyleProvider =
      CreateOpaqueButtonPointerStyleProvider();

  return secondaryActionButton;
}

- (UIButton*)createTertiaryButton {
  DCHECK(self.tertiaryActionString);
  UIButton* tertiaryActionButton = [UIButton buttonWithType:UIButtonTypeSystem];
  [tertiaryActionButton addTarget:self
                           action:@selector(didTapTertiaryActionButton)
                 forControlEvents:UIControlEventTouchUpInside];
  [tertiaryActionButton setTitle:self.tertiaryActionString
                        forState:UIControlStateNormal];
  tertiaryActionButton.contentEdgeInsets =
      UIEdgeInsetsMake(kButtonVerticalInsets, 0, kButtonVerticalInsets, 0);
  [tertiaryActionButton setBackgroundColor:[UIColor clearColor]];
  UIColor* titleColor = [UIColor colorNamed:kBlueColor];
  [tertiaryActionButton setTitleColor:titleColor forState:UIControlStateNormal];
  tertiaryActionButton.titleLabel.font =
      [UIFont preferredFontForTextStyle:UIFontTextStyleBody];
  tertiaryActionButton.titleLabel.adjustsFontForContentSizeCategory = NO;
  tertiaryActionButton.translatesAutoresizingMaskIntoConstraints = NO;
  tertiaryActionButton.accessibilityIdentifier =
      kConfirmationAlertTertiaryActionAccessibilityIdentifier;

  tertiaryActionButton.pointerInteractionEnabled = YES;
  tertiaryActionButton.pointerStyleProvider =
      CreateOpaqueButtonPointerStyleProvider();

  return tertiaryActionButton;
}

@end
